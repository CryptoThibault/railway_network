# Local Task Tracker

Keep this file in English and local to the workspace. Move tasks between sections as priorities change.

## Current: establish reliable library tests

- [x] Rebuild the library and establish an execution baseline: 24 examples compile, 21 exit successfully, 3 interactive examples remain unexecuted.
- [x] Rebuild and run the railway demo successfully against the rebuilt library.
- [ ] Make the library test runner return a failure when compilation or execution fails, including individual runs and Valgrind checks.
- [ ] Add assertions for existing behavior; console output and successful execution alone are insufficient.
- [ ] Add regression tests for each confirmed defect before fixing it.
- [ ] After each fix, rerun library checks and the railway demo; use sanitizers for memory and concurrency changes.

## Deferred: library fixes

- [ ] Factory: preserve type safety instead of casting unchecked `void*` values; remove unnecessary allocations and ensure exception safety.
- [ ] Registry: define pointer lifetime and concurrency guarantees; protect or restrict removal while references remain in use.
- [ ] Loader: reject malformed numbers and handle Unicode escapes correctly. Confirmed cases: `1.2.3` is accepted as `1.2`; `"\u0041"` becomes `u0041` instead of `A`.
- [ ] Pool: prevent shallow ownership copies and double deletion; handle construction failures and borrowed handle lifetimes.
- [ ] WorkerPool: synchronize pool allocation/release, review condition-variable coordination, and define task exception handling.
- [ ] Thread: define safe destruction, start/join behavior, and movement while running.
- [ ] DataBuffer: constrain raw serialization to suitable types and define format portability.
- [ ] Client/Server: handle partial sends, socket cleanup, and concurrent access to shared connection state.
- [ ] Strengthen automated coverage for interactive I/O and networking examples.

## Deferred: railway architecture

- [x] Build `ftpp`, `railway_core`, and the console demo as separate CMake targets; keep `make` as a wrapper.
- [x] Register existing noninteractive examples and railway checks with CTest.
- [x] Support an explicit scenario path and bundle the default scenario for build/install execution outside the repository.

- [ ] Replace umbrella-header dependencies with specific includes and forward declarations.
- [ ] Validate train-type lookups and scenario data before constructing usable objects.
- [ ] Encapsulate changes to train location instead of exposing unrestricted mutable `Board` access.
- [ ] Introduce an explicit time step and a simulation coordinator.
- [ ] Define stable ownership for registered objects and train callbacks capturing `this`.

## Future features

- [ ] Complete journeys with segment changes, braking, and station stops.
- [ ] Simulate multiple trains on a shared clock with explicit track occupation rules.
- [ ] Add route optimization with a defined cost and constraints.
- [ ] Add a desktop graphical interface independent of the simulation logic.
