# engine/devtools

## Scope
Developer tooling and in-engine debugging:
- ImGui integration bridge
- debug views, runtime inspectors, profiling overlays
- non-shipping diagnostics helpers

## Ownership
- **Primary owner:** Tools team.
- **Reviewers:** rendering owner for UI backend integration changes.

## Extension points
- Add tooling panels as isolated components with clear data-provider interfaces.
- Keep devtool dependencies optional and gated for non-production builds.
- Route instrumentation through shared telemetry interfaces rather than direct subsystem hooks.

## Parallel-work rules
- Tool UI code should not own business/runtime state; consume via read/write APIs.
- Any frame-loop impact should be measured and documented.
