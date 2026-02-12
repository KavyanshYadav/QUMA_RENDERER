add_library(engine_samples_bundle INTERFACE)
add_library(Engine::samples_bundle ALIAS engine_samples_bundle)
target_link_libraries(
  engine_samples_bundle
  INTERFACE
    Engine::core_runtime
    Engine::platform_runtime
    Engine::render_runtime
    Engine::modules_runtime
)

install(TARGETS engine_samples_bundle EXPORT EngineTargets)
