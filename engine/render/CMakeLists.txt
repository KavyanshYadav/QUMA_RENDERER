add_library(engine_render_contract INTERFACE)
add_library(Engine::render_contract ALIAS engine_render_contract)
target_link_libraries(
  engine_render_contract
  INTERFACE
    Engine::build_options
    Engine::warnings
    Engine::core_contract
    Engine::platform_contract
)

add_library(engine_render_backend_opengl INTERFACE)
add_library(Engine::render_backend_opengl ALIAS engine_render_backend_opengl)
target_link_libraries(engine_render_backend_opengl INTERFACE engine_render_contract)

add_library(engine_render_runtime INTERFACE)
add_library(Engine::render_runtime ALIAS engine_render_runtime)
target_link_libraries(engine_render_runtime INTERFACE engine_render_contract engine_render_backend_opengl)

install(TARGETS engine_render_contract engine_render_backend_opengl engine_render_runtime EXPORT EngineTargets)
