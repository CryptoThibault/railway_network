# Railway Network

A railway simulation written in C++20 using object-oriented design.
Development guidelines are in [AGENTS.md](AGENTS.md).

## Build and run

Requires Linux, a C++20-compatible compiler, CMake 3.20 or later, and Make.
Run from the project root:

```sh
make
./build/bin/network
```

The Makefile delegates to CMake. Equivalent commands:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/bin/network
```

All build artifacts stay in `build/`; the executable is `build/bin/network`.
`make run` builds and launches the demo. `make clean` removes object files and
their dependency files, keeping executables. `make fclean` also removes the demo
and library example executables. Both retain static libraries, resources, and
CMake configuration in the selected `BUILD_DIR`. `make re` runs `fclean` then builds.
Use `make BUILD_DIR=build/release BUILD_TYPE=Release` for a separate release build.

CMake copies the sample to `build/share/railway_network/data.json`. The executable
locates it relative to its own location using Linux `/proc/self/exe`, so it can run
from any working directory. To use another scenario:

```sh
./build/bin/network /path/to/scenario.json
```

An explicit relative scenario path is resolved from the working directory.
To install the executable and sample together, use
`cmake --install build --prefix /path/to/install`, then run `/path/to/install/bin/network`.

## Tests

```sh
make test
```

This builds the project and runs CTest with failure output. After building, use
`ctest --test-dir build --output-on-failure` to run tests directly.
CTest runs the 21 noninteractive library examples and three railway checks
(bundled scenario, explicit scenario, and missing file). Interactive examples are
compiled but not automatically run. Library examples remain smoke checks, not a
complete regression suite. Use `-DBUILD_TESTING=OFF` when configuring CMake to omit tests.

## Current behavior

- Loads stations, segments, train types, trains, and journeys from JSON into typed registries.
- Links segments to their stations and assigns each train a type and an initial station.
- Uses a state machine for train behavior and a separate motion component for speed and distance.
- Runs a checked demo on the first train and a segment connected to its station:
  idle, waiting, acceleration (600 updates), cruising (20), braking (600), and return to idle.
  Each update represents one simulated second; no real-time pauses are used.

The demo checks speed bounds, acceleration/braking direction, cruising distance,
stopping, and rejection of an invalid transition. It prints a summary per phase
and returns a nonzero exit code on failure. The fixed phase lengths target the
sample scenario; they are not an automatic driving controller.

The sample contains three stations, two segments, five train types, and three trains.
Journeys are loaded but not yet used. Segment changes, scheduled stops, and automatic
routing are not implemented. Input validation is still partial.

## Project structure

- `inc/` and `src/`: railway classes, initialization, motion, and console output.
- `data.json`: sample scenario with `stations`, `segments`, `trainTypes`, `trains`, and `journey` arrays.
- `lib/`: local utility library, including JSON loading, registries, factories, and state machines.
- `lib/test/`: utility library examples and checks; no dedicated railway test suite yet.

CMake builds `ftpp` as a utility library, `railway_core` as the reusable railway
logic, and `network` as the console demo with loading and printing. A future GUI
can link to `railway_core` without using the console entry point. Header coupling
and global registries still need improvement; no graphical framework is selected.
The standalone `lib/Makefile` remains available but is not used by the root build.

## Next steps

- Simulate multiple trains on a shared simulation clock.
- Support complete journeys and optimized routes.
- Add a graphical desktop interface.

This README will grow alongside the implementation.
