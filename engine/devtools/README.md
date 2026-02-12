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

## Current tool packages
- `imgui_tools/` exposes `ImguiToolsSuite`, which consumes abstract service interfaces for metrics, module lifecycle, renderer debug state, and configuration persistence.
- The suite emits panel models for frame/memory dashboards, module load-unload-reload controls, renderer resource/draw statistics, and live-edited configuration values.
- Devtools are compiled only when `ENGINE_ENABLE_DEVTOOLS=ON`; production builds can disable this option while retaining engine contracts.
- With `ENGINE_AUTO_FETCH_IMGUI=ON`, CMake will fetch/build ImGui automatically when needed.

## Parallel-work rules
- Tool UI code should not own business/runtime state; consume via read/write APIs.
- Any frame-loop impact should be measured and documented.
