add_library(engine_core_contract INTERFACE)
add_library(Engine::core_contract ALIAS engine_core_contract)
target_link_libraries(engine_core_contract INTERFACE Engine::build_options Engine::warnings)

add_library(engine_core_runtime INTERFACE)
add_library(Engine::core_runtime ALIAS engine_core_runtime)
target_link_libraries(engine_core_runtime INTERFACE engine_core_contract)

install(TARGETS engine_core_contract engine_core_runtime EXPORT EngineTargets)
