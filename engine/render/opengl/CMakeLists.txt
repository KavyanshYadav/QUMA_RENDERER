add_library(engine_render_backend_opengl STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGlBackendStub.cpp
)
add_library(Engine::render_backend_opengl ALIAS engine_render_backend_opengl)

target_link_libraries(engine_render_backend_opengl PUBLIC engine_render_contract)

target_include_directories(
  engine_render_backend_opengl
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

find_package(OpenGL QUIET)

if(OpenGL_FOUND)
  target_sources(engine_render_backend_opengl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/OpenGlRenderBackend.cpp)
  target_link_libraries(engine_render_backend_opengl PRIVATE OpenGL::GL)
  set(ENGINE_RENDER_HAS_OPENGL ON PARENT_SCOPE)
else()
  message(STATUS "OpenGL not found: engine_render_backend_opengl will be a stub target")
endif()

install(TARGETS engine_render_backend_opengl EXPORT EngineTargets)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
