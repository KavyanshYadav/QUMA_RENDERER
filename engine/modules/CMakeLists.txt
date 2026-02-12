add_library(engine_modules_contract INTERFACE)
add_library(Engine::modules_contract ALIAS engine_modules_contract)
target_link_libraries(
  engine_modules_contract
  INTERFACE
    Engine::build_options
    Engine::warnings
    Engine::core_contract
)

add_library(engine_modules_runtime INTERFACE)
add_library(Engine::modules_runtime ALIAS engine_modules_runtime)
target_link_libraries(engine_modules_runtime INTERFACE engine_modules_contract)

install(TARGETS engine_modules_contract engine_modules_runtime EXPORT EngineTargets)
