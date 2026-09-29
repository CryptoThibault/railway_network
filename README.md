# Railway Network

Railway Network brings together stations, tracks, and trains to simulate train
movement, from acceleration to braking. Future features will include network
visualization, complete journeys, and route planning.

## Console build

Requires Linux, Make, and a C++20 compiler. The original Makefiles remain unchanged:

```sh
make
./network
```

Run the console simulation from the repository root so it can load `data.json`.
The executable is generated at the root. `make clean` removes its object files,
`make fclean` also removes `network`, and `make re` rebuilds it.
The root Makefile calls `lib/Makefile` to build the utility archive when missing.

## Desktop build

CMake orchestrates the existing Makefile build and builds the graphical application
separately. Requires CMake 3.20 or later, pkg-config, and X11/Xft development
headers (`libx11-dev` and `libxft-dev` on Debian/Ubuntu), in addition to the console build dependencies. Tests also require
Python 3; configure with `-DBUILD_TESTING=OFF` to omit them.

```sh
cmake -S . -B build
cmake --build build --parallel
./build/network_window
```

Each CMake build invokes the root Makefile, which checks whether `network` needs
rebuilding. CMake does not reimplement compilation of the simulation or library.
The desktop executable stays in `build/`; the console executable stays at the root.
CMake clean affects its graphical build artifacts; use `make clean` or `make fclean`
for the console build.

`network_window` opens a resizable 1024 × 720 dashboard in an X11 or XWayland
desktop session. Fixed cards show speed, train state, and simulated time. The route
panel shows departure, destination, distance, completed legs, and a train drawing moving along the track. Values update in place, with smooth fonts and double-buffered drawing.

- Click Start or press Enter to launch the repository's `network` executable.
- Click Stop or press Enter again to stop the simulation. You can then restart it.
  The window stays responsive and shows running, stopped, or failure status.
- Click Pause or press Space to freeze the train and clock. Click Resume or press
  Space again to continue without catching up on paused time. Pause is available
  once the running simulation has sent its first update.
- Select **1h–5h** in the clock card, or press **1–5**, to choose how many simulated
  hours pass per real minute (60×, 120×, 180×, 240×, or 300×). Changes apply during
  a run or while paused, without restarting. The selection is kept for the next run.
- Stopping preserves the last displayed values; starting again resets the dashboard.
  Launch and process errors appear in the status line.
- Press Escape or use the desktop close button to close the window and stop its
  simulation process.

The launcher uses the repository path recorded by CMake, so it works from another
working directory. Reconfigure after moving the repository. Simulation graphics
are not displayed yet; graphical code remains separate from simulation code.
The launcher reads the console process through a pipe and displays the latest complete
telemetry update. Partial or invalid lines do not replace valid values. Raw console
output stays available when running `./network` directly.

## Simulation and checks

The first train makes one trip along its first connected segment, Paris–Lyon
in the sample. It waits 30 simulated seconds before departure, accelerates, cruises,
and brakes to a stop at the destination. Arrival ends the simulation automatically;
the dashboard keeps the final values and Start becomes available again. Other trains and loaded journeys remain
inactive. Routing across multiple segments is not implemented.

Playback defaults to **60×** and can be increased to **300×: five simulated hours
per real minute**. For console use, run `./network --speed 300` (any integer from
60 to 300 is accepted).

A steady-clock accumulator advances motion in fixed 0.1-second simulated steps,
processed in batches about every 16 ms. Changing speed affects the clock rate, not
the physics step. Dashboard updates are limited to about ten per second, with
additional updates for state or control changes. The dashboard shows:

- Simulated time, train ID, and train type.
- State: Waiting, Accelerating, Cruising, or Braking.
- Speed in km/h and position along the current leg in kilometres.
- Departure, destination, and completed legs.

Position is measured from the departure station and reaches the segment length on arrival.
Braking uses a simple stopping-distance estimate; once stopped within two steps of
travel at the speed limit, the train is placed at the station. This is an approximate
trip controller, not a timetable or signalling system. The sample's 427 km leg
takes roughly 85 real seconds at 60× or 17 seconds at 300×, including acceleration,
braking, and station dwell.
Press Ctrl+C to stop a console run.

For a deterministic run without real-time delays, use `./network --steps 200000`
(each step represents 0.1 simulated seconds). Normal launch runs until arrival or a manual stop.

After a CMake build, run the tests:

```sh
ctest --test-dir build --output-on-failure
```

No display is required. Tests cover automatic termination after one trip, states, speed and
position bounds, identical physics at all five presets, clock pacing at 60× and
300×, live speed changes, argument validation, output capture, stopping,
restarting, pause/resume without clock catch-up, telemetry parsing, and launch
failures. To check the window manually, click Start, watch the fixed values and
train drawing advance, then try the speed presets, Pause, Resume, Stop, resizing,
and Escape.
Library examples remain available through `lib/Makefile`.

## Project structure

Development guidelines are in [AGENTS.md](AGENTS.md).

- `inc/` and `src/`: railway logic, data loading, console presentation, and simulation entry point.
- `gui/`: desktop entry point and window implementation, built only by CMake.
- `lib/`: generic utilities and their standalone Makefile.
- `data.json`: sample scenario containing three stations, two segments, five train
  types, and three trains.

## Next steps

- Add a graphical railway map alongside the train dashboard.
- Simulate multiple trains on a shared clock.
- Support complete journeys and route planning.
