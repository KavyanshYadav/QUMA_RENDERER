# third_party

## Scope
Dependency intake and governance for external libraries.

## Ownership
- **Primary owner:** Build/Infra team.
- **Security reviewer:** required for all new dependencies.

## Intake strategy
- Prefer package-manager or pinned source mirror with reproducible versions.
- Record license, version, source URL, checksum, and update policy for each dependency.
- Keep local patches minimal and documented with rebase instructions.

## Extension points
- Add one folder per dependency with metadata (`README`, `LICENSE`, patch notes).
- Define integration wrappers in engine code rather than exposing raw third-party APIs widely.

## Parallel-work rules
- No implicit dependency upgrades; use explicit PRs with changelog and impact summary.
