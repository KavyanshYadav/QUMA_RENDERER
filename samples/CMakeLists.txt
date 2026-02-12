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
engine_resolve_glad(ENGINE_GLAD_TARGET)
engine_resolve_imgui(ENGINE_IMGUI_TARGET)
find_package(OpenGL QUIET)

if(ENGINE_SAMPLES_SDL2_TARGET AND OpenGL_FOUND AND ENGINE_GLAD_TARGET AND ENGINE_IMGUI_TARGET)
  get_target_property(ENGINE_IMGUI_INCLUDE_DIRS ${ENGINE_IMGUI_TARGET} INTERFACE_INCLUDE_DIRECTORIES)
  list(GET ENGINE_IMGUI_INCLUDE_DIRS 0 ENGINE_IMGUI_ROOT_DIR)
  string(REGEX REPLACE "^\\$<BUILD_INTERFACE:([^>]+)>$" "\\1" ENGINE_IMGUI_ROOT_DIR "${ENGINE_IMGUI_ROOT_DIR}")

  add_executable(engine_sample_opengl_triangle
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl_triangle/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl_triangle/MeshRenderEngine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/opengl_triangle/ObjLoader.cpp
    ${ENGINE_IMGUI_ROOT_DIR}/backends/imgui_impl_sdl2.cpp
    ${ENGINE_IMGUI_ROOT_DIR}/backends/imgui_impl_opengl3.cpp
  )

  target_link_libraries(
    engine_sample_opengl_triangle
    PRIVATE
      Engine::samples_bundle
      ${ENGINE_SAMPLES_SDL2_TARGET}
      ${ENGINE_GLAD_TARGET}
      ${ENGINE_IMGUI_TARGET}
      OpenGL::GL
  )

  target_include_directories(
    engine_sample_opengl_triangle
    PRIVATE
      ${ENGINE_IMGUI_ROOT_DIR}
      ${ENGINE_IMGUI_ROOT_DIR}/backends
  )

  target_compile_features(engine_sample_opengl_triangle PRIVATE cxx_std_20)
else()
  message(STATUS "Skipping engine_sample_opengl_triangle: SDL2, OpenGL, GLAD, and/or ImGui not available")
endif()

install(TARGETS engine_samples_bundle EXPORT EngineTargets)
