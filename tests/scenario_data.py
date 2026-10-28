import json
import math
from itertools import combinations
from pathlib import Path
import sys

root = Path(sys.argv[1]).resolve()
data = json.loads((root / "data.json").read_text())
stations = {station["name"]: station for station in data["stations"]}
assert len(stations) == len(data["stations"])
assert set(stations) == {"Paris", "Lyon", "Marseille", "Berlin", "Frankfurt", "Zurich", "Rome"}
for station in stations.values():
    assert all(math.isfinite(station[axis]) for axis in ("x", "y"))
    assert 200000 < station["x"] < 1600000 and 5000000 < station["y"] < 7100000


def orientation(a, b, c):
    return ((b["x"] - a["x"]) * (c["y"] - a["y"])
            - (b["y"] - a["y"]) * (c["x"] - a["x"]))


def on_segment(a, b, c):
    return (orientation(a, b, c) == 0
            and min(a["x"], b["x"]) <= c["x"] <= max(a["x"], b["x"])
            and min(a["y"], b["y"]) <= c["y"] <= max(a["y"], b["y"]))


def geographic(station):
    return (2 * math.atan(math.exp(station["y"] / 6378137)) - math.pi / 2,
            station["x"] / 6378137)


edges = set()
neighbours = {name: set() for name in stations}
for segment in data["segments"]:
    a, b = segment["a"], segment["b"]
    assert a in stations and b in stations and a != b
    key = frozenset((a, b))
    assert key not in edges, ("Duplicate segment", a, b)
    edges.add(key)
    neighbours[a].add(b)
    neighbours[b].add(a)
    assert math.isfinite(segment["length"]) and segment["length"] > 0
    assert math.isfinite(segment["maxSpeed"]) and 0 < segment["maxSpeed"] <= 320 / 3.6 + 1e-6
    latitude_a, longitude_a = geographic(stations[a])
    latitude_b, longitude_b = geographic(stations[b])
    haversine = (math.sin((latitude_b - latitude_a) / 2) ** 2
                 + math.cos(latitude_a) * math.cos(latitude_b)
                 * math.sin((longitude_b - longitude_a) / 2) ** 2)
    distance = 2 * 6371000 * math.asin(math.sqrt(haversine))
    assert segment["length"] >= distance, ("Rail length shorter than air distance", a, b)
    for name, point in stations.items():
        if name not in (a, b):
            assert not on_segment(stations[a], stations[b], point), ("Unmodelled junction", a, b, name)

for first, second in combinations(data["segments"], 2):
    if {first["a"], first["b"]} & {second["a"], second["b"]}:
        continue
    a, b = stations[first["a"]], stations[first["b"]]
    c, d = stations[second["a"]], stations[second["b"]]
    crossing = (orientation(a, b, c) * orientation(a, b, d) < 0
                and orientation(c, d, a) * orientation(c, d, b) < 0)
    touching = any((on_segment(a, b, c), on_segment(a, b, d),
                    on_segment(c, d, a), on_segment(c, d, b)))
    assert not crossing and not touching, ("Intersecting segments", first, second)

visited = set()
pending = [next(iter(stations))]
while pending:
    name = pending.pop()
    if name not in visited:
        visited.add(name)
        pending.extend(neighbours[name] - visited)
assert visited == set(stations), "Disconnected railway network"
print(f"Scenario checks passed: {len(stations)} cities, {len(edges)} noncrossing corridors")
