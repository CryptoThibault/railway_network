# Railway Network

Follow a train from **Paris to Marseille via Lyon**, with acceleration, cruising,
braking, and passenger stops. A live dashboard shows the train moving between
stations, its speed and state, and adjustable playback from 60× to 300×.

## Build and run

Requires Linux, Make, and a C++20 compiler. For the console simulation:

```sh
make
./network
```

Run from the repository root to load `data.json`. The executable is `./network`.
Use Ctrl+C to stop, or `./network --speed 300` for faster playback. The `--speed`
option accepts integers from 60 to 300.

The desktop dashboard also requires CMake 3.20+, pkg-config, X11/Xft development
headers (`libx11-dev` and `libxft-dev` on Debian/Ubuntu), and an X11 or XWayland
session. Tests require Python 3; configure with `-DBUILD_TESTING=OFF` to omit them.

```sh
cmake -S . -B build
cmake --build build --parallel
./build/network_window
```

CMake invokes the existing root Makefile to build `network`, then builds the
separate dashboard in `build/`. Both Makefiles remain unchanged. `make clean`
removes console objects; `make fclean` also removes `network`. CMake's clean target
cleans its own artifacts. The library retains its standalone build and examples.

## Dashboard controls

| Control | Action |
| --- | --- |
| Start / Stop or Enter | Start a fresh trip or stop the current one |
| Pause / Resume or Space | Freeze or resume the train and clock without catching up |
| 1h–5h buttons or keys 1–5 | Select simulated hours per real minute: 60×–300× |
| Escape or window close | Close the dashboard and stop its simulation process |

Speed can change during playback or while paused and is retained for the next run.
Pause becomes available after the first simulation update. Stopping preserves the
last values; restarting resets them.

Fixed cards show speed, train state, simulated time, and the passenger-stop
countdown. The schematic route shows all three named stations, platforms, and a
train with a streamlined cab, carriage, and pantograph. Overall progress covers
736 km; the active segment's distance is shown separately.

The dashboard launches the root executable using the repository path recorded by
CMake, so it works from another directory. Reconfigure after moving the repository.
Launch and process failures appear in the status line. Console output remains
available by running `./network` directly.

## Simulation behavior

The first train follows two segments: Paris–Lyon (427 km), then Lyon–Marseille
(309 km). At **each station**, it spends **10 simulated minutes in `Waiting`** for
passengers. It accelerates, cruises, and brakes on each segment. The simulation
ends after the final passenger stop in Marseille; it does not start a return trip.
Each station stop takes 10 real seconds at 60× or 2 seconds at 300×.

Motion uses fixed 0.1-second simulated steps, processed in batches about every
16 ms using a steady clock. Playback speed changes the clock rate, not the physics.
Telemetry is emitted about ten times per second and on state or control changes.
The dashboard validates complete telemetry lines, draws through a back buffer,
and skips unchanged mouse-hover redraws. It sleeps until a window event when no
process or pending output needs attention.

Braking uses a stopping-distance estimate. Once stopped within two steps of travel
at the speed limit, the train is placed at the station. This is an approximate
controller, without signalling or collision handling. The route is selected
explicitly; other trains and loaded journey schedules remain inactive.

## Checks

```sh
ctest --test-dir build --output-on-failure
```

Tests run without a display and cover both segments, all three passenger stops,
final termination, speed and position bounds, identical physics across presets,
live speed changes, pause/resume, telemetry parsing, and process lifecycle.

For a deterministic console run without real-time delays:

```sh
./network --steps 200000
```

The step count is an upper limit; the run ends earlier if the route is complete.
For a visual check, launch the dashboard, resize it, and exercise Start, Pause,
Resume, the speed presets, and Stop while watching the station labels and train.

## Project structure

- `inc/`, `src/`: data loading, railway logic, and console entry point.
- `gui/`: dashboard, process control, and telemetry decoding.
- `tests/`: simulation and launcher checks.
- `lib/`: generic utilities with their own Makefile and examples.
- `data.json`: sample stations, segments, train types, trains, and journey schedules.

Development guidelines are in [AGENTS.md](AGENTS.md).

## Planned features

- A geographical railway map.
- Multiple active trains on a shared clock.
- Journey schedules and automatic route planning.
