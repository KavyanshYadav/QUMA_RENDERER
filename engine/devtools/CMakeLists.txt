add_library(engine_devtools_contract INTERFACE)
add_library(Engine::devtools_contract ALIAS engine_devtools_contract)
target_link_libraries(
  engine_devtools_contract
  INTERFACE
    Engine::build_options
    Engine::warnings
    Engine::core_contract
    Engine::render_contract
    Engine::modules_contract
)

if(ENGINE_ENABLE_DEVTOOLS)
  add_subdirectory(imgui_tools)
endif()

add_library(engine_devtools_runtime INTERFACE)
add_library(Engine::devtools_runtime ALIAS engine_devtools_runtime)
target_link_libraries(engine_devtools_runtime INTERFACE engine_devtools_contract)

if(ENGINE_ENABLE_DEVTOOLS)
  target_link_libraries(engine_devtools_runtime INTERFACE Engine::devtools_imgui_tools)
endif()

install(TARGETS engine_devtools_contract engine_devtools_runtime EXPORT EngineTargets)
