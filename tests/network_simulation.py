import copy
import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

root = Path(sys.argv[1]).resolve()
scenario = json.loads((root / "data.json").read_text())


def run(data):
    with tempfile.TemporaryDirectory() as directory:
        Path(directory, "data.json").write_text(json.dumps(data))
        return subprocess.run([str(root / "network"), "--once"], cwd=directory,
                              capture_output=True, text=True, timeout=10)


def seconds(line):
    hours, minutes, seconds = map(int, line[:8].split(":"))
    return hours * 3600 + minutes * 60 + seconds


fixture = copy.deepcopy(scenario)
fixture["trains"] = fixture["trains"][:2]
fixture["trains"][0].update(start="Paris", end="Marseille", departure=28800, pause=300)
fixture["trains"][1].update(start="Marseille", end="Paris", departure=28800, pause=120)
result = run(fixture)
assert result.returncode == 0, result.stderr
lines = result.stdout.splitlines()
assert len(lines) == 10, result.stdout
for train_id, pause in ((1, 300), (2, 120)):
    events = [line for line in lines if f"| Train {train_id} " in line]
    assert len(events) == 4, events
    assert seconds(events[0]) == 28800, events
    assert "Stopped at Lyon" in events[1], events
    assert "Departed Lyon" in events[2], events
    assert abs(seconds(events[2]) - seconds(events[1]) - pause) <= 1, events
    assert "Completed at" in events[3], events
    assert seconds(events[3]) > seconds(events[2]), events
assert "Simulation finished" in lines[-1]

fixture["trains"][1]["departure"] = 32400
result = run(fixture)
assert result.returncode == 0, result.stderr
assert re.search(r"09:00:00 .*Train 2 .*Departed Marseille", result.stdout), result.stdout

for city in ("Berlin", "Frankfurt", "Zurich", "Rome"):
    expanded = copy.deepcopy(scenario)
    expanded["trains"] = expanded["trains"][:2]
    expanded["trains"][0].update(start=city, end="Paris")
    result = run(expanded)
    assert result.returncode == 0, (city, result.stderr)
    assert "Train 1 " in result.stdout and "Completed at Paris" in result.stdout
    assert "Simulation finished" in result.stdout

fixture["segments"] = []
result = run(fixture)
assert result.returncode != 0 and "No route" in result.stderr, result.stderr

fixture["trains"] = []
result = run(fixture)
assert result.returncode == 0 and "Simulation finished" in result.stdout

result = run(scenario)
assert result.returncode == 0, result.stderr
lines = result.stdout.splitlines()
for train_id, train in enumerate(scenario["trains"], 1):
    events = [line for line in lines if f"| Train {train_id} " in line]
    assert len(events) >= 2, (train_id, events)
    assert seconds(events[0]) == train["departure"], events
    assert f"Completed at {train['end']}" in events[-1], events
berlin = next(line for line in lines if "ICE:BERLIN-MARSEILLE" in line and "Route:" in line)
assert "Berlin -> Frankfurt -> Paris -> Lyon -> Marseille" in berlin, berlin
assert len(lines) < 100, result.stdout
print("Network simulation checks passed")
