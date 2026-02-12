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
target_include_directories(
  engine_render_contract
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(opengl)

add_library(engine_render_runtime STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/RenderBackendFactory.cpp
)
add_library(Engine::render_runtime ALIAS engine_render_runtime)
target_link_libraries(engine_render_runtime PUBLIC engine_render_contract engine_render_backend_opengl)

if(ENGINE_RENDER_HAS_OPENGL)
  target_compile_definitions(engine_render_runtime PUBLIC ENGINE_RENDER_HAS_OPENGL=1)
else()
  target_compile_definitions(engine_render_runtime PUBLIC ENGINE_RENDER_HAS_OPENGL=0)
endif()

install(TARGETS engine_render_contract engine_render_runtime EXPORT EngineTargets)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
