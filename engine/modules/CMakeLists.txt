add_library(engine_modules_contract INTERFACE)
add_library(Engine::modules_contract ALIAS engine_modules_contract)
target_link_libraries(
  engine_modules_contract
  INTERFACE
    Engine::build_options
    Engine::warnings
    Engine::core_contract
)
target_include_directories(
  engine_modules_contract
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(
  engine_modules_contract
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/modules/IModule.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/modules/ModuleAbi.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/modules/ModuleContract.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/modules/ModuleManager.hpp
)

add_library(engine_modules_runtime INTERFACE)
add_library(Engine::modules_runtime ALIAS engine_modules_runtime)
target_link_libraries(engine_modules_runtime INTERFACE engine_modules_contract)

install(TARGETS engine_modules_contract engine_modules_runtime EXPORT EngineTargets FILE_SET HEADERS)
