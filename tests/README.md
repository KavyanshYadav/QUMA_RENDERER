# tests

## Scope
Automated validation for engine behavior:
- unit tests for subsystem-local logic
- integration tests across subsystem boundaries
- contract tests for interfaces/backends/modules

## Ownership
- **Primary owner:** Quality/Verification team.
- **Contributors:** all subsystem teams must add coverage for new contracts.

## Extension points
- Organize tests by layer (`unit/`, `integration/`, `contracts/`) as suites are introduced.
- Add shared fixtures/utilities under a common testing support area.
- Treat contract tests as required for any new backend implementation.

## Parallel-work rules
- New public interfaces require corresponding contract tests.
- Regressions must include reproduction coverage before merge.
