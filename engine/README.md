# Engine Layer

## Purpose
`engine/` contains all production runtime source for the engine itself. It is split into focused domains so multiple contributors can work without stepping on each other.

## Subsystems
- `core/` – bootstrapping, lifecycle, runtime services, and shared configuration contracts.
- `platform/` – OS and hardware adaptation layers (windowing, input, filesystem, timers).
- `render/` – rendering abstractions and concrete graphics API backends.
- `modules/` – feature/plugin model for optional or hot-swappable runtime capabilities.
- `devtools/` – editor/debug overlays and developer-facing instrumentation.

## Ownership model
- **Engine Architecture Group** owns cross-cutting contracts and boundary rules.
- **Subsystem owners** own implementation details inside each subfolder.
- Cross-subsystem changes require at least one reviewer from each affected subsystem.

## Extension points
- Introduce new public engine contracts under the owning subsystem, then document lifecycle and threading assumptions in that subsystem README.
- Prefer adding new capabilities via `modules/` when functionality is optional or product-specific.
- Keep subsystem interfaces stable; version any breaking interface change and publish migration notes.
