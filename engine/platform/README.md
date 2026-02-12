# engine/platform

## Scope
Platform abstraction boundary:
- window creation and event pumping
- input devices and mapping
- backend/system interfaces (filesystem, timing, threading, etc.)
- per-platform implementations (Win/Linux/macOS, etc.)

## Ownership
- **Primary owner:** Platform team.
- **Reviewers required for new platform:** one platform owner + one core owner.

## Extension points
- Define backend-agnostic interfaces first (`IWindowSystem`, `IInputSystem`, etc.), then provide per-platform adapters.
- Keep platform-specific code isolated to implementation folders.
- Introduce capability flags for feature detection instead of compile-time scattering.

## Current structure
- Public contracts are exposed under `engine/platform/include/engine/platform/`.
- SDL implementation is isolated under `engine/platform/sdl/`; renderer/core code should only depend on `Engine::platform_contract` or `Engine::platform_runtime`.
- Backend construction flows through `createPlatformBackend(...)`, allowing future replacement with GLFW/native without changing renderer/core modules.

## Parallel-work rules
- New platform implementation must not change existing interface semantics without an ADR.
- Shared interface changes require coordinated updates in all supported implementations.
