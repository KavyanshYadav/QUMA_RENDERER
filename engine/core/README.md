# engine/core

## Scope
Core runtime orchestration:
- application lifecycle and main loop
- startup/shutdown sequencing
- service registry / dependency wiring
- config loading and environment profiles

## Ownership
- **Primary owner:** Runtime/Core team.
- **Secondary reviewers:** all subsystem leads for lifecycle-impacting changes.

## Extension points
- Add startup phases through explicit lifecycle hooks (e.g., pre-init, init, post-init, shutdown).
- Register new services via a typed service registry contract rather than direct singletons.
- Extend configuration through versioned schema and defaults in one place.

## Parallel-work rules
- Do not introduce subsystem-specific logic here; keep policies generic.
- Any public core interface change must include upgrade notes for downstream subsystems.
