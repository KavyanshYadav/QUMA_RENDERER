# Architecture & Layering Rules

This document defines the dependency direction and boundary rules for `QumaRenderer`.

## Layer model

Top-level runtime layers (lowest to highest):

1. **Core (`engine/core`)**
   - Runtime lifecycle, shared utilities, and cross-cutting contracts.
   - Must not depend on higher-level subsystems.
2. **Platform (`engine/platform`)**
   - OS adaptation (window/input/clipboard/monitor abstractions and backend factories).
   - Depends on core contracts only.
3. **Render (`engine/render`)**
   - Graphics abstractions and backend-facing render logic.
   - Depends on core and platform-facing contracts as needed.
4. **Modules (`engine/modules`)**
   - Plugin/module lifecycle, module contracts, hot-swap policy, and descriptor validation.
   - May depend on core/platform/render contracts, but never on devtools internals.
5. **Devtools (`engine/devtools`)**
   - Editor/debug overlays and instrumentation hooks.
   - Can depend on all lower layers but cannot become a runtime hard dependency of them.

## Dependency direction

Allowed dependency direction is **downward only**:

- `devtools -> modules/render/platform/core`
- `modules -> render/platform/core`
- `render -> platform/core`
- `platform -> core`
- `core -> (none inside engine)`

### Forbidden dependencies

- Any lower layer depending on a higher layer.
- Cross-layer access through concrete implementation headers when an interface exists.
- Direct coupling between sibling implementations (e.g., one backend implementation directly linking another backend implementation).

## Boundary rules

- Depend on public headers from `include/` first; keep private headers local to each subsystem.
- Introduce cross-layer interactions via explicit interfaces/contracts.
- Keep platform and renderer backends replaceable (factory/contract-based selection).
- Keep optional features in modules rather than expanding core APIs for product-specific behavior.

## Change control checklist

Before merging architectural changes:

- Confirm dependency direction remains valid.
- Document any new public contract and ownership.
- Add tests at the lowest layer where behavior is enforceable.
- Add an RFC/ADR when introducing a new layer rule, major contract, or compatibility policy.
