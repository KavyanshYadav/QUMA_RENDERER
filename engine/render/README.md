# engine/render

## Scope
Rendering contracts and backends:
- renderer API abstraction (`IRenderBackend`, `IRenderDevice`, `ICommandContext`)
- frame graph extension hooks (`IFrameGraphHook`)
- backend/resource handles for buffers, textures, shaders, and pipelines
- concrete backend implementation, starting with OpenGL

## Ownership
- **Primary owner:** Rendering team.
- **Cross-review required:** platform owner when touching context/surface integration.

## Extension points
- Add rendering features behind backend-neutral interfaces before backend specialization.
- Keep backend-specific headers (OpenGL/Vulkan/DirectX) isolated under backend folders.
- Select backend through `RenderBackendFactory` + `--render-backend=<name>` to avoid core rewrites when adding new APIs.

## Current structure
- Public render contracts live under `engine/render/include/engine/render/`.
- OpenGL backend code is isolated under `engine/render/opengl/`; only the backend implementation sees OpenGL headers.
- Runtime backend creation is centralized in `createRenderBackend(...)`, with configuration/CLI selection via `selectRenderBackendType(...)`.

## Parallel-work rules
- Avoid leaking OpenGL types into public renderer interfaces.
- Backend-specific optimizations must preserve behavior under the shared render API contract.
