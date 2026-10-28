import copy
import json
from pathlib import Path
import subprocess
import sys
import tempfile

root = Path(sys.argv[1]).resolve()
train = json.loads((root / "data.json").read_text())["trains"][0]
train.update(start="A", end="D", departure=0, pause=0, maxSpeed=100,
             mass=100, engineForce=10000, brakeForce=10000, friction=0)
scenario = {
    "stations": [{"name": name, "x": index * 1000, "y": 0}
                 for index, name in enumerate(("A", "B", "C", "D"))],
    "segments": [
        {"a": "A", "b": "D", "length": 6000, "maxSpeed": 20},
        {"a": "A", "b": "B", "length": 4000, "maxSpeed": 100},
        {"a": "B", "b": "D", "length": 4000, "maxSpeed": 100},
        {"a": "A", "b": "C", "length": 1000, "maxSpeed": 100},
        {"a": "C", "b": "B", "length": 1000, "maxSpeed": 100},
    ],
    "trains": [train],
}


def run(data):
    with tempfile.TemporaryDirectory() as directory:
        Path(directory, "data.json").write_text(json.dumps(data))
        return subprocess.run([str(root / "network"), "--once"], cwd=directory,
                              capture_output=True, text=True, timeout=10)


def route(data, expected):
    result = run(data)
    assert result.returncode == 0, result.stderr
    planned = [line.split("Route: ", 1)[1] for line in result.stdout.splitlines()
               if "Route: " in line]
    assert planned == [expected], result.stdout
    assert "Completed at " + data["trains"][0]["end"] in result.stdout
    return result.stdout


route(scenario, "A -> C -> B -> D")
scenario["trains"][0]["pause"] = 30
route(scenario, "A -> B -> D")
scenario["trains"][0]["pause"] = 300
route(scenario, "A -> D")
scenario["trains"][0].update(pause=30, maxSpeed=10)
route(scenario, "A -> D")
scenario["trains"][0].update(start="D", end="A", pause=0, maxSpeed=100)
route(scenario, "D -> B -> C -> A")

handling = copy.deepcopy(scenario)
handling["trains"][0].update(start="A", end="D", pause=0)
handling["segments"] = [
    {"a": "A", "b": "D", "length": 1200, "maxSpeed": 30},
    {"a": "A", "b": "B", "length": 400, "maxSpeed": 100},
    {"a": "B", "b": "D", "length": 400, "maxSpeed": 100},
]
route(handling, "A -> B -> D")
handling["trains"][0]["engineForce"] = 50
route(handling, "A -> D")
handling["trains"][0].update(engineForce=10000, brakeForce=50)
route(handling, "A -> D")

parallel = copy.deepcopy(scenario)
parallel["trains"][0].update(start="A", end="D", pause=0)
parallel["segments"] = [
    {"a": "A", "b": "D", "length": 6000, "maxSpeed": 1},
    {"a": "A", "b": "D", "length": 6000, "maxSpeed": 100},
]
output = route(parallel, "A -> D")
completed = next(line for line in output.splitlines() if "Completed at D" in line)
hours, minutes, seconds = map(int, completed[:8].split(":"))
assert hours * 3600 + minutes * 60 + seconds < 1000, output

for key, value in (("maxSpeed", 0), ("maxSpeed", -1), ("length", 0), ("length", -1)):
    invalid = copy.deepcopy(scenario)
    invalid["segments"][0][key] = value
    result = run(invalid)
    assert result.returncode != 0 and "Invalid segment parameters" in result.stderr

unreachable = copy.deepcopy(scenario)
unreachable["segments"] = [segment for segment in scenario["segments"]
                           if "A" not in (segment["a"], segment["b"])]
result = run(unreachable)
assert result.returncode != 0 and "No route" in result.stderr
print("Pathfinder checks passed")
