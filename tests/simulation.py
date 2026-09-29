import math
import pathlib
import re
import subprocess
import sys
import time

root = pathlib.Path(sys.argv[1])
pattern = re.compile(
    r'Time (\d+)h(\d+)m(\d+)s .* State: (\w+) \| Speed: ([\d.]+) km/h'
    r' \| Segment: (.+?) -> (.+?) \| Position: ([\d.]+) / ([\d.]+) km \| Legs: (\d+)'
)
result = subprocess.run([str(root / 'network'), '--steps', '200000'], cwd=root,
                        capture_output=True, text=True, check=True, timeout=15)
rows = [pattern.search(line) for line in result.stdout.splitlines()]
rows = [row.groups() for row in rows if row]
assert rows, 'No train telemetry'
assert {'Waiting', 'Accelerating', 'Cruising', 'Braking'} <= {row[3] for row in rows}
assert int(rows[-1][9]) == 1, 'Train did not finish exactly one trip'
assert rows[-1][3] == 'Idle' and float(rows[-1][4]) == 0, 'Train did not stop on arrival'
assert float(rows[-1][7]) == float(rows[-1][8]), 'Train did not reach the destination'
assert {row[5:7] for row in rows} == {('Paris', 'Lyon')}
previous_time = -1
for hours, minutes, seconds, state, speed, departure, destination, position, length, legs in rows:
    elapsed = int(hours) * 3600 + int(minutes) * 60 + int(seconds)
    assert elapsed >= previous_time, 'Simulation clock went backwards'
    previous_time = elapsed
    assert math.isfinite(float(speed)) and 0 <= float(speed) <= 320.1
    assert 0 <= float(position) <= float(length), 'Position left the segment'
    if state == 'Waiting':
        assert float(speed) == 0, 'Train moved while waiting'
assert 4800 < previous_time < 5400, 'Simulation did not terminate at the first arrival'
assert all(int(row[9]) <= 1 for row in rows), 'A return trip started'
short = subprocess.run([str(root / 'network'), '--steps', '1000'], cwd=root,
                       capture_output=True, text=True, check=True, timeout=3)
assert 'Time 0h1m40s' in short.stdout, 'Fixed simulation step drifted'

for scale in [60, 120, 180, 240, 300]:
    fast = subprocess.run([str(root / 'network'), '--steps', '200000', '--speed', str(scale)],
                          cwd=root, capture_output=True, text=True, check=True, timeout=5)
    fast_rows = [pattern.search(line) for line in fast.stdout.splitlines()]
    assert [row.groups() for row in fast_rows if row] == rows, 'Speed changed simulation physics'
    assert f'| {scale}x |' in fast.stdout, 'Telemetry reports the wrong speed'

for scale in [60, 300]:
    process = subprocess.Popen([str(root / 'network'), '--speed', str(scale)], cwd=root,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        start = time.monotonic()
        time.sleep(1.1)
        process.terminate()
        output, errors = process.communicate(timeout=3)
        elapsed_real = time.monotonic() - start
        assert process.returncode == 0, errors
        clocks = [pattern.search(line) for line in output.splitlines()]
        clocks = [row for row in clocks if row]
        last = clocks[-1]
        simulated = int(last[1]) * 3600 + int(last[2]) * 60 + int(last[3])
        assert 0.75 * scale <= simulated / elapsed_real <= 1.15 * scale, 'Incorrect real-time pacing'
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()

for arguments in [['--steps', '0'], ['--steps', 'invalid'], ['--unknown'], ['--speed', '59'],
                  ['--speed', '301'], ['--speed', 'invalid'], ['--speed']]:
    invalid = subprocess.run([str(root / 'network'), *arguments], cwd=root,
                             capture_output=True, text=True, timeout=3)
    assert invalid.returncode != 0, 'Invalid arguments were accepted'
