# Project Guidelines

These rules apply throughout the repository. See [README.md](README.md) for current
behavior and [TASKS.md](TASKS.md) for priorities.

## Scope and documentation

- Do not modify `lib/`; it is outside the current development scope.
- Treat `gui/` as a demo, not the target interface.
- Keep project content in English. Do not add comments to code files.
- Keep the README short and describe implemented behavior as features are added.
- Keep only broad objectives in TASKS; only the active objective gets subtasks.
  Select one objective at a time and keep future plans separate from current behavior.

## Design and implementation

- Use C++20 and focused classes; prefer composition, explicit dependencies, and
  simple solutions. Avoid speculative abstractions and premature optimization.
- Separate data loading, simulation, route planning, and presentation. Keep the
  simulation independent of the GUI and railway logic in `inc/` and `src/`.
- Use the development style in `lib/` throughout the project, including naming,
  formatting, and class organization. Do not introduce custom or anonymous
  namespaces; use `static` for file-local helpers and qualify standard names with `std::`.
- Prefer RAII, value semantics, and const-correctness. Make ownership explicit and
  keep borrowed references valid.
- Validate external data and check lookups before dereferencing pointers.
- Keep changes focused and preserve unrelated work. After functional changes,
  build and run relevant checks; add focused tests as behavior grows.

## Git workflow

- When the user says "push", run `git add . && git commit -m "..." && git push`.
- Write the commit message in English and describe the main feature added.
