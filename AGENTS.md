# Project Guidelines

These rules apply throughout the repository. See [README.md](README.md) for current behavior.

## Language and documentation

- Keep all project content in English, including code, documentation, and messages.
- Do not add comments to code files. Explain behavior and design in the README.
- Keep documentation concise and update it as features are implemented.
- Separate existing functionality from future plans.

## Architecture

- Use modern C++20 and object-oriented design, following SOLID, KISS, and YAGNI.
- Give each class a clear responsibility and prefer composition over inheritance.
- Keep data loading, simulation logic, route planning, and presentation separate.
- Keep the simulation independent of the future graphical interface.
- Prefer explicit dependencies and introduce abstractions only when needed.
- Keep `lib/` generic; railway-specific logic belongs in `inc/` and `src/`.

## Development

- Do not introduce custom or anonymous namespaces. Use `static` for file-local helper functions and keep standard library names qualified with `std::`.
- Follow the existing naming and formatting style; use clear, descriptive names.
- Prefer RAII, value semantics, and const-correctness. Make ownership explicit and keep borrowed references valid.
- Validate external data and check lookups before dereferencing pointers.
- Keep changes focused and preserve unrelated work.
- Build and run relevant checks after functional changes; add focused tests as behavior grows.
- Avoid premature optimization and speculative architecture.

## Git workflow

- When the user says "push", run `git add . && git commit -m "..." && git push`.
- Write the commit message in English and describe the main feature added.
