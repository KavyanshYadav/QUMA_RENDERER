# Coding Standard

This standard applies to C++ engine code and supporting CMake/docs in this repository.

## Naming

### C++ types and interfaces

- Types: `PascalCase` (`ModuleManager`, `PlatformBackendFactory`).
- Abstract interfaces: prefix with `I` (`IModule`, `IPlatformBackend`).
- Enum types: `PascalCase`; enumerators scoped and descriptive.

### Functions and variables

- Functions/methods: `camelCase`.
- Local variables/parameters: `camelCase`.
- Member fields: `m_` prefix + `camelCase`.
- Constants: `kPascalCase` for internal constants, `UPPER_SNAKE_CASE` for macros only.

### Files

- Public headers in `include/` aligned to namespace hierarchy.
- Source files mirror header names where possible (`Foo.hpp` / `Foo.cpp`).

## Error handling

- Validate inputs and invariants at subsystem boundaries.
- Use return values/status objects for recoverable runtime failures.
- Reserve exceptions for truly exceptional, non-local failure paths.
- Include enough context to diagnose failures (module name, backend, API version, operation phase).

## Logging

- Log at boundaries and state transitions (startup/shutdown/load/unload/fallback).
- Use structured, searchable log messages:
  - subsystem
  - action
  - outcome
  - key identifiers (module/backend/version)
- Avoid noisy per-frame logs in hot paths without sampling/rate limiting.

## Assertions

- Use assertions for programmer errors and impossible states.
- Do not use assertions as user-facing runtime validation.
- Keep assertions side-effect free.
- Pair critical assertions with tests that exercise the same invariants.

## Formatting and linting

- Formatting must be deterministic and CI-enforced.
- Treat lint warnings as merge blockers for touched code unless explicitly waived.
- Keep includes ordered and remove dead code before merge.
