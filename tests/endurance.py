import json
from pathlib import Path
import re
import subprocess
import sys
import tempfile

root = Path(sys.argv[1]).resolve()
days = int(sys.argv[2]) if len(sys.argv) > 2 else 7
assert days >= 2
scenario = json.loads((root / "data.json").read_text())


def timestamp(line):
    hours, minutes, seconds = map(int, line[:8].split(":"))
    day = int(re.search(r"\| Day (\d+)", line)[1])
    return (day - 1) * 86400 + hours * 3600 + minutes * 60 + seconds


with tempfile.TemporaryDirectory() as directory:
    Path(directory, "data.json").write_text(json.dumps(scenario))
    result = subprocess.run([str(root / "network"), "--days", str(days)], cwd=directory,
                            capture_output=True, text=True, timeout=max(60, days * 10))
    assert result.returncode == 0, result.stderr
    output = result.stdout.splitlines()
    assert f"Simulation finished | 00:00:00 | Day {days + 1}" == output[-1], output[-1]
    files = list(Path(directory, "log").glob("*.txt"))
    assert len(files) == len(scenario["trains"]), len(files)
    records_by_train = {}
    for path in files:
        records = path.read_text().splitlines()
        assert records, path
        train_id = int(re.search(r"\| Train (\d+)", records[0])[1])
        assert train_id not in records_by_train
        assert all(f"| Train {train_id} " in line for line in records)
        assert all("State: Scheduled" not in line for line in records)
        times = [timestamp(line) for line in records]
        assert times == sorted(times), path
        records_by_train[train_id] = records
    for train_id, train in enumerate(scenario["trains"], 1):
        events = [line for line in output if f"| Train {train_id} " in line]
        starts = [timestamp(line) for line in events if "| Route:" in line]
        assert starts == [train["departure"] + day * 86400 for day in range(days)], (train_id, starts)
        arrivals = [timestamp(line) for line in events if "Completed at " in line]
        assert days - 1 <= len(arrivals) <= days, (train_id, arrivals)
        rows = records_by_train[train_id]
        completed = [timestamp(line) for line in rows if "State: Completed" in line]
        assert completed == arrivals, (train_id, completed, arrivals)
        for line in rows:
            time = timestamp(line)
            run = max(index for index, start in enumerate(starts) if start <= time)
            assert run >= len(arrivals) or time <= arrivals[run], (train_id, line)
        for start in starts:
            first = next(line for line in rows if timestamp(line) == start)
            distance = float(re.search(r"Distance: ([0-9.]+) m", first)[1])
            first_step_bound = train["engineForce"] / train["mass"] * 0.1 ** 2 + 0.0051
            assert 0.0 <= distance <= first_step_bound and "State: Accelerating" in first, first
print(f"Endurance checks passed: {len(scenario['trains'])} services over {days} days")
