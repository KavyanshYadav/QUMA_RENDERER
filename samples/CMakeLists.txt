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

engine_resolve_sdl2(ENGINE_SAMPLES_SDL2_TARGET)
find_package(OpenGL QUIET)

if(ENGINE_SAMPLES_SDL2_TARGET AND OpenGL_FOUND)
  add_executable(engine_sample_opengl_triangle
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl_triangle/main.cpp
  )

  target_link_libraries(
    engine_sample_opengl_triangle
    PRIVATE
      Engine::samples_bundle
      ${ENGINE_SAMPLES_SDL2_TARGET}
      OpenGL::GL
  )

  target_compile_features(engine_sample_opengl_triangle PRIVATE cxx_std_20)
else()
  message(STATUS "Skipping engine_sample_opengl_triangle: SDL2 and/or OpenGL not available")
endif()

install(TARGETS engine_samples_bundle EXPORT EngineTargets)
