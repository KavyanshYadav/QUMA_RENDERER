add_library(engine_platform_contract INTERFACE)
add_library(Engine::platform_contract ALIAS engine_platform_contract)
target_link_libraries(engine_platform_contract INTERFACE Engine::build_options Engine::warnings Engine::core_contract)
target_include_directories(
  engine_platform_contract
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(
  engine_platform_contract
  INTERFACE
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/IClipboardSystem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/IInputSystem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/IMonitorSystem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/IPlatformBackend.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/IWindowSystem.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/PlatformBackendFactory.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/platform/PlatformTypes.hpp
)

set(ENGINE_PLATFORM_HAS_SDL OFF)
add_subdirectory(sdl)

add_library(engine_platform_runtime STATIC)
add_library(Engine::platform_runtime ALIAS engine_platform_runtime)
target_sources(engine_platform_runtime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/PlatformBackendFactory.cpp)
target_include_directories(engine_platform_runtime PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/sdl/include)
target_link_libraries(engine_platform_runtime PUBLIC engine_platform_contract PRIVATE engine_platform_backend_sdl)
target_compile_definitions(engine_platform_runtime PRIVATE ENGINE_PLATFORM_HAS_SDL=$<BOOL:${ENGINE_PLATFORM_HAS_SDL}>)

install(TARGETS engine_platform_contract engine_platform_runtime EXPORT EngineTargets FILE_SET HEADERS)
