# engine/render

## Scope
Rendering contracts and backends:
- renderer API abstraction (device, swapchain, command submission)
- render graph or pipeline orchestration contracts
- concrete backend implementation, starting with OpenGL

## Ownership
- **Primary owner:** Rendering team.
- **Cross-review required:** platform owner when touching context/surface integration.

## Extension points
- Add rendering features behind backend-neutral interfaces before backend specialization.
- Place OpenGL-specific code in a dedicated backend area and keep generic API clean.
- Future backends (e.g., Vulkan/Metal/D3D) should implement the same core contracts.

## Parallel-work rules
- Avoid leaking OpenGL types into public renderer interfaces.
- Backend-specific optimizations must preserve behavior under the shared render API contract.
