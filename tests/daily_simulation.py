import copy
import json
from pathlib import Path
import re
import signal
import subprocess
import sys
import tempfile
import time

root = Path(sys.argv[1]).resolve()
train = json.loads((root / "data.json").read_text())["trains"][0]
train.update(name="DAILY", start="A", end="C", departure=0, pause=60,
             mass=100, maxSpeed=20, engineForce=100, brakeForce=100, friction=0)
scenario = {
    "stations": [{"name": name, "x": index * 20000, "y": 0}
                 for index, name in enumerate(("A", "B", "C"))],
    "segments": [
        {"a": "A", "b": "B", "length": 20000, "maxSpeed": 20},
        {"a": "B", "b": "C", "length": 20000, "maxSpeed": 20},
    ],
    "trains": [train],
}


def run(data, arguments):
    with tempfile.TemporaryDirectory() as directory:
        Path(directory, "data.json").write_text(json.dumps(data))
        result = subprocess.run([str(root / "network"), *arguments], cwd=directory,
                                capture_output=True, text=True, timeout=20)
        files = {path.name: path.read_text().splitlines()
                 for path in Path(directory, "log").glob("*.txt")}
        return result, files


def log_seconds(line):
    hours, minutes, seconds = map(int, line[:8].split(":"))
    day = int(re.search(r"\| Day (\d+)", line)[1])
    return (day - 1) * 86400 + hours * 3600 + minutes * 60 + seconds


result, files = run(scenario, ["--days", "3"])
assert result.returncode == 0, result.stderr
assert set(files) == {"DAILY.txt"}
lines = files["DAILY.txt"]
assert all("| Train 1 (DAILY)" in line for line in lines)
assert all("State:" in line and "Speed:" in line and "Distance:" in line for line in lines)
assert sum("Departed A" in line for line in result.stdout.splitlines()) == 3
assert sum("Completed at C" in line for line in result.stdout.splitlines()) == 3
samples = []
for day in range(1, 4):
    records = [line for line in lines if f"| Day {day} |" in line]
    assert 3 <= len(records) < 12, records
    assert records[0].startswith(f"00:00:00 | Day {day}"), records
    assert "Segment: A -> B" in records[0], records
    assert "State: Completed" in records[-1] and "Speed: 0.00 m/s" in records[-1]
    assert sum("State: Completed" in line for line in records) == 1
    arrival = next(line for line in result.stdout.splitlines()
                   if f"| Day {day} |" in line and "Completed at C" in line)
    assert log_seconds(records[-1]) == log_seconds(arrival)
    assert all(log_seconds(line) % 300 == 0 for line in records[1:-1])
    assert all(log_seconds(line) <= log_seconds(arrival) for line in records)
    sample = next(line for line in records if line.startswith("00:05:00"))
    assert "Segment: A -> B" in sample and "Speed: 20.00 m/s" in sample
    samples.append(sample.split("| State:", 1)[1])
assert samples[0] == samples[1] == samples[2], samples

paused = copy.deepcopy(scenario)
paused["trains"][0]["pause"] = 600
result, files = run(paused, ["--once"])
assert result.returncode == 0, result.stderr
pause_record = next(line for line in files["DAILY.txt"] if line.startswith("00:20:00"))
assert "State: Waiting" in pause_record and "Station: B" in pause_record, pause_record

short = copy.deepcopy(scenario)
short["trains"][0]["end"] = "B"
short["segments"] = [{"a": "A", "b": "B", "length": 100, "maxSpeed": 20}]
result, files = run(short, ["--once"])
assert result.returncode == 0 and len(files["DAILY.txt"]) == 2
assert "State: Completed" in files["DAILY.txt"][-1]

night = copy.deepcopy(scenario)
night["trains"][0]["departure"] = 85800
result, files = run(night, ["--days", "3"])
assert result.returncode == 0, result.stderr
assert result.stdout.count("Departed A") == 3, result.stdout
assert result.stdout.count("Completed at C") == 2, result.stdout
midnight = next(line for line in files["DAILY.txt"] if line.startswith("00:00:00 | Day 2"))
assert "Segment: A -> B" in midnight and "Speed: 20.00 m/s" in midnight, midnight

long_trip = copy.deepcopy(scenario)
long_trip["trains"][0].update(end="B", maxSpeed=10)
long_trip["segments"] = [{"a": "A", "b": "B", "length": 900000, "maxSpeed": 10}]
result, files = run(long_trip, ["--days", "3"])
assert result.returncode == 0, result.stderr
departures = [line for line in result.stdout.splitlines() if "Departed A" in line]
assert len(departures) == 2 and "Day 1" in departures[0] and "Day 3" in departures[1], departures

multi = copy.deepcopy(scenario)
reverse = dict(multi["trains"][0], name="REVERSE", start="C", end="A", departure=300)
multi["trains"].append(reverse)
result, files = run(multi, ["--days", "1"])
assert result.returncode == 0 and set(files) == {"DAILY.txt", "REVERSE.txt"}
assert all("| Train 1 " in line for line in files["DAILY.txt"])
assert all("| Train 2 " in line for line in files["REVERSE.txt"])
assert any("Segment: C -> B" in line for line in files["REVERSE.txt"])
assert log_seconds(files["REVERSE.txt"][0]) == 300
assert "Station: C" not in files["REVERSE.txt"][0]
assert sum("State: Completed" in line for line in files["REVERSE.txt"]) == 1

colliding = copy.deepcopy(multi)
colliding["trains"][0]["name"] = "A/B"
colliding["trains"][1]["name"] = "A:B"
result, files = run(colliding, ["--once"])
assert result.returncode == 0 and set(files) == {"A_B.txt", "A_B_2.txt"}, files.keys()
assert all("| Train 1 " in line for line in files["A_B.txt"])
assert all("| Train 2 " in line for line in files["A_B_2.txt"])

with tempfile.TemporaryDirectory() as directory:
    Path(directory, "data.json").write_text(json.dumps(scenario))
    process = subprocess.Popen([str(root / "network")], cwd=directory,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        deadline = time.monotonic() + 5
        log = Path(directory, "log", "DAILY.txt")
        while not log.exists() and process.poll() is None and time.monotonic() < deadline:
            time.sleep(0.01)
        assert log.exists() and process.poll() is None
        time.sleep(0.1)
        process.send_signal(signal.SIGINT)
        output, errors = process.communicate(timeout=5)
        assert process.returncode == 0 and "Simulation stopped" in output, (output, errors)
        assert "Playback: 1200x" in output, output
        assert "Day 2" not in output and len(log.read_text().splitlines()) == 1, output
        log.write_text("obsolete record\n")
        stale = Path(directory, "log", "old", "nested.txt")
        stale.parent.mkdir()
        stale.write_text("obsolete nested file")
        Path(directory, "log", "train_999.txt").write_text("obsolete train")
        result = subprocess.run([str(root / "network"), "--once"], cwd=directory,
                                capture_output=True, text=True, timeout=5)
        assert result.returncode == 0, result.stderr
        records = log.read_text().splitlines()
        assert len(records) > 1 and records[0].startswith("00:00:00 | Day 1")
        assert "obsolete" not in log.read_text()
        assert not stale.parent.exists()
        assert set(Path(directory, "log").iterdir()) == {log}
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate()

with tempfile.TemporaryDirectory() as directory:
    Path(directory, "data.json").write_text(json.dumps(scenario))
    process = subprocess.Popen([str(root / "network"), "--speed", "600"], cwd=directory,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        log = Path(directory, "log", "DAILY.txt")
        deadline = time.monotonic() + 5
        while process.poll() is None and time.monotonic() < deadline:
            if log.exists() and len(log.read_text().splitlines()) >= 2:
                break
            time.sleep(0.01)
        assert process.poll() is None and log.exists()
        process.send_signal(signal.SIGINT)
        output, errors = process.communicate(timeout=5)
        assert process.returncode == 0, errors
        assert "Playback: 600x" in output and "Simulation running" not in output, output
        assert "Simulation stopped" in output
        records = log.read_text().splitlines()
        assert len(records) >= 2 and log_seconds(records[1]) == 300, records
    finally:
        if process.poll() is None:
            process.kill()
            process.communicate()

with tempfile.TemporaryDirectory() as directory:
    Path(directory, "data.json").write_text(json.dumps(scenario))
    Path(directory, "log").write_text("not a directory")
    result = subprocess.run([str(root / "network"), "--once"], cwd=directory,
                            capture_output=True, text=True, timeout=5)
    assert result.returncode != 0 and "network:" in result.stderr

for arguments in (["--days", "0"], ["--speed", "-1"], ["--days", "x"],
                  ["--once", "--days", "1"], ["--days"], ["--data-only", "--once"]):
    result = subprocess.run([str(root / "network"), *arguments], cwd=root,
                            capture_output=True, text=True, timeout=5)
    assert result.returncode != 0, arguments
print("Daily simulation and train log checks passed")
