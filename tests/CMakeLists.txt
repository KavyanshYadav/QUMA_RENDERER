add_library(engine_tests_contracts INTERFACE)
add_library(Engine::tests_contracts ALIAS engine_tests_contracts)
target_link_libraries(
  engine_tests_contracts
  INTERFACE
    Engine::core_contract
    Engine::platform_contract
    Engine::render_contract
    Engine::modules_contract
)

install(TARGETS engine_tests_contracts EXPORT EngineTargets)
