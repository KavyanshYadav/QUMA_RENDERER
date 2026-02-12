# engine/modules

## Scope
Feature module/plugin system:
- module discovery and registration
- load/unload lifecycle for optional features
- runtime module communication contracts

## Ownership
- **Primary owner:** Gameplay/Feature systems team.
- **Core reviewer required:** for module lifecycle or safety changes.

## Extension points
- Implement modules behind the explicit `IModule` lifecycle contract (`onLoad`, `onStart`, `onStop`, `onUnload`).
- Use explicit module manifests (`ModuleDescriptor`) for dependencies, conflicts, swap policy, and API compatibility.
- Prefer event/message contracts over direct cross-module coupling.

## Runtime compatibility + loading
- Dynamic libraries expose stable C ABI entry points:
  - `CreateModule`
  - `DestroyModule`
  - `QueryModuleApiVersion`
- `ModuleManager` validates all loaded module descriptors before activation:
  - rejects incompatible engine/module API versions
  - rejects missing dependencies
  - rejects declared conflicts
  - rejects cyclic dependency graphs

## Hot-swap policy by module category
The swap policy must be declared per module capability:

### Runtime swappable (`RuntimeSwappable`)
Safe for live replacement when the module owns only isolated algorithmic state.
Typical examples:
- post-process algorithms
- effect compositors
- deterministic gameplay rule evaluators with serialized state handoff

### Restart required (`RestartRequired`)
Requires engine restart due to global side effects or OS/resource ownership.
Typical examples:
- window manager backend
- graphics device/swapchain backend
- low-level input backend integrations

## Parallel-work rules
- Modules should remain independently buildable and testable.
- Breaking module interface changes require version bump + migration notes.
