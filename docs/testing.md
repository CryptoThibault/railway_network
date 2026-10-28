# Regression checks

Build the console with `make`. CMake 3.20+, Python 3, pkg-config, and X11/Xft
headers are needed to configure the full build, including the old GUI demo.

```sh
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

| Check | Coverage |
| --- | --- |
| `loading` | Scenario structure, required fields, numeric types, station names, references, coordinate bounds, and invalid values |
| `scenario_data` | Connected seven-city network, noncrossing schematic segments, positive distances and speed limits |
| `pathfinder` | Route changes with speed, train capabilities, acceleration, braking, pauses, parallel tracks, and missing routes |
| `network_simulation` | Shared clock, scheduled departures, intermediate stops, all sample journeys, and bounded console output |
| `daily_simulation` | Repeated days, overnight journeys, skipped departures, restart counters, paced playback, cancellation, log boundaries, filenames, and stale-log cleanup |
| `simulation_core` | C++ state transitions, event order/times, physical bounds, analytic motion estimates, resets, and 360 route comparisons against exhaustive search |
| `headers` | Each project header compiles on its own under C++20 with warnings treated as errors |

Python checks can also run as `python3 tests/<name>.py .`. The core test is
`./build/simulation_core_test`. Checks use temporary scenario directories and do
not overwrite the default scenario or its logs. Seeds are fixed for repeatability.
Older `simulation.py` and `simulation_process.cpp` belong to the previous GUI
protocol and are not part of the current console regression suite.

## Multi-day endurance run

```sh
python3 tests/endurance.py . 7
```

This optional longer check runs the sample services across seven days in a
temporary directory. It checks daily departures, resets, overnight arrivals,
monotonic timestamps, and logs confined to active journeys. Pass another day
count as the final argument. It is separate from the shorter default CTest suite.

## Memory and undefined behavior

Instrument the C++ core checks without changing the normal console executable:

```sh
cmake -S . -B build/sanitized \
  -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer -g' \
  -DCMAKE_EXE_LINKER_FLAGS='-fsanitize=address,undefined'
cmake --build build/sanitized --target simulation_core_test --parallel
./build/sanitized/simulation_core_test
```

Project core sources are instrumented; `lib/` keeps its existing build.
LeakSanitizer needs an execution environment without ptrace restrictions.
