# Railway Network

C++20 railway simulation with a shared clock, daily schedules, acceleration,
braking, intermediate stops, and train logs. The sample schedules are fictional.

## Build and run

Requires Linux, Make, and a C++20 compiler. Run from the repository root:

```sh
make
./network
```

Playback defaults to 1200× with fixed 0.1-second steps. Use `--speed 300` to change
speed and Ctrl+C to stop. `--once` runs every service once; `--days 3` runs through
three calendar days. Both finite modes run without real-time delays.
`--data-only` loads the scenario without simulating or clearing logs.

Services repeat at the same time each day, restarting at the origin without a
simulated return trip. Overnight journeys continue normally; departures missed
while travelling are skipped. Track occupation and collisions are not handled.

Routes minimize estimated duration using Dijkstra: acceleration, train/segment
speed limits, braking to rest, and pauses. Estimates assume constant forces and
ignore resistance. Actual motion still applies resistance.

`Scheduled`, intermediate `Waiting`, movement phases, and `Completed` are distinct
states. Simulation events drive console departures, stops, and arrivals; there are
no periodic progress messages. Registry objects must remain stable during a run.

## Data and logs

`data.json` contains stations, segments, and trains. Loading checks field types,
required values, names, unique stations, references, and coordinate bounds.
All physical data use SI units: m, s, kg, N, and m/s. City-centre markers use
EPSG:3857 projected meters; segment lengths are separate railway travel distances.
Sources, estimates, schema, and model details are in [Data reference](docs/data.md).

`log/<train-name>.txt` records departure, arrival, and five-minute simulated samples
between them, including station pauses. Records contain day/time, state, location,
speed, and trip distance. Names are sanitized and collisions get an ID suffix.
`log/` is recreated for every simulation, ignored by Git, and removed by `make clean`.
Files stay open during a run. Console events use Logger; registry details use
`Logger::debug()`, enabled by setting the level in `main` to `Debug`.

## Structure and checks

- `inc/`, `src/`: loading, train model, pathfinding, simulation, and presentation.
- `tests/`: persistent Python and C++ regression checks.
- `gui/`: previous demo dashboard; its protocol is not supplied by this executable.
- `lib/`: existing generic library, outside the development scope.

With CMake 3.20+, Python 3, pkg-config, and X11/Xft development headers:

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

See [Testing](docs/testing.md) for coverage and sanitizer commands,
[AGENTS.md](AGENTS.md) for development rules, and [TASKS.md](TASKS.md) for priorities.
The long-term goal is an interactive European railway map.
