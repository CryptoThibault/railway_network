import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(sys.argv[1]).resolve()
debug = "setLevel(Logger::Level::Debug)" in (root / "src/main.cpp").read_text()
scenario = json.loads((root / "data.json").read_text())


def run(data):
    with tempfile.TemporaryDirectory() as directory:
        Path(directory, "data.json").write_text(json.dumps(data))
        return subprocess.run([str(root / "network"), "--data-only"], cwd=directory,
                              capture_output=True, text=True, timeout=5)


result = run(scenario)
assert result.returncode == 0, result.stderr
expected_output = (f"Stations ({len(scenario['stations'])})",
                 f"Segments ({len(scenario['segments'])})",
                 f"Trains ({len(scenario['trains'])})",
                 f"Paris -> {scenario['trains'][0]['end']} | departure: 08:00:00",
                 "Marseille -> Paris | departure: 09:00:00",
                 "pause: 300 s per intermediate station", str(scenario["stations"][0]["y"]))
if debug:
    for expected in expected_output:
        assert expected in result.stdout, result.stdout
else:
    assert result.stdout == "", result.stdout

for field, value in (("start", "Unknown"), ("end", "Unknown"),
                     ("end", "Paris"), ("mass", 0), ("pause", -1),
                     ("departure", 86400), ("departure", 0.5)):
    invalid = copy.deepcopy(scenario)
    invalid["trains"][0][field] = value
    result = run(invalid)
    assert result.returncode != 0 and "network:" in result.stderr, (field, result)

for invalid_root in ([], None, 17, "invalid"):
    result = run(invalid_root)
    assert result.returncode != 0 and "Scenario must be an object" in result.stderr

for collection in ("stations", "segments", "trains"):
    for invalid_value in (None, {}, "invalid", [1]):
        invalid = copy.deepcopy(scenario)
        invalid[collection] = invalid_value
        result = run(invalid)
        assert result.returncode != 0 and "network:" in result.stderr, (collection, invalid_value)

for collection, key, values in (
        ("stations", "name", ("", " ", " Paris", "Paris ", "Paris\n", 12, None)),
        ("trains", "name", ("", " ", "\ttrain", "train\n", 12, None)),
        ("stations", "x", ("261467", True, None, 20037509, -20037509)),
        ("stations", "y", ("6250025", False, None, 20037509, -20037509)),
        ("trains", "mass", ("400000", True, None)),
        ("segments", "maxSpeed", ("83.3", True, None))):
    for value in values:
        invalid = copy.deepcopy(scenario)
        invalid[collection][0][key] = value
        result = run(invalid)
        assert result.returncode != 0 and "network:" in result.stderr, (collection, key, value)

for collection, key in (("stations", "name"), ("stations", "x"),
                        ("trains", "name"), ("trains", "departure"), ("segments", "length")):
    invalid = copy.deepcopy(scenario)
    del invalid[collection][0][key]
    result = run(invalid)
    assert result.returncode != 0 and f"Missing field: {key}" in result.stderr, result.stderr

invalid = copy.deepcopy(scenario)
invalid["stations"].append(dict(invalid["stations"][0], x=0))
result = run(invalid)
assert result.returncode != 0 and "Duplicate station name: Paris" in result.stderr

for raw in ('{"stations":[{"name":"Paris","x":1e309,"y":0}],"segments":[],"trains":[]}',
            '{"stations":[{"name":"Paris","x":0,"y":-1e309}],"segments":[],"trains":[]}'):
    with tempfile.TemporaryDirectory() as directory:
        Path(directory, "data.json").write_text(raw)
        result = subprocess.run([str(root / "network"), "--data-only"], cwd=directory,
                                capture_output=True, text=True, timeout=5)
        assert result.returncode != 0 and "network:" in result.stderr

valid = copy.deepcopy(scenario)
valid["stations"][0].update(x=-1000, y=-1000)
result = run(valid)
assert result.returncode == 0, result.stderr

empty = copy.deepcopy(scenario)
empty["trains"] = []
result = run(empty)
assert result.returncode == 0
if debug:
    assert "Trains (0)" in result.stdout
else:
    assert result.stdout == "", result.stdout
print("Loading checks passed")
