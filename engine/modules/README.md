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
- Implement modules behind a stable `IModule` contract.
- Use explicit module manifests for dependencies and compatibility.
- Prefer event/message contracts over direct cross-module coupling.

## Parallel-work rules
- Modules should remain independently buildable and testable.
- Breaking module interface changes require version bump + migration notes.
