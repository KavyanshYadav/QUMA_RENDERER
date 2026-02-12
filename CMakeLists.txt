cmake_minimum_required(VERSION 3.23)

project(QumaRenderer VERSION 0.1.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENGINE_BUILD_TESTS "Build engine test targets" ON)
option(ENGINE_BUILD_SAMPLES "Build engine sample targets" ON)
option(ENGINE_ENABLE_IMGUI "Enable ImGui-backed developer tools" ON)
option(ENGINE_ENABLE_DEVTOOLS "Build developer tooling modules" ON)
option(ENGINE_ENABLE_PROFILING "Enable profiling instrumentation" OFF)
option(ENGINE_AUTO_FETCH_SDL2 "Automatically fetch/build SDL2 when not found" ON)
option(ENGINE_AUTO_FETCH_IMGUI "Automatically fetch/build ImGui for devtools" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

if(CMAKE_CONFIGURATION_TYPES)
  foreach(config ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER "${config}" config_upper)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${config_upper} "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
  endforeach()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(EngineCompilerOptions)
include(EngineCompilerSmokeTest)
include(EngineThirdParty)

add_library(engine_build_options INTERFACE)
add_library(Engine::build_options ALIAS engine_build_options)

if(ENGINE_ENABLE_IMGUI)
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_IMGUI=1)
else()
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_IMGUI=0)
endif()

if(ENGINE_ENABLE_DEVTOOLS)
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_DEVTOOLS=1)
else()
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_DEVTOOLS=0)
endif()

if(ENGINE_ENABLE_PROFILING)
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_PROFILING=1)
else()
  target_compile_definitions(engine_build_options INTERFACE ENGINE_ENABLE_PROFILING=0)
endif()

add_library(engine_warnings INTERFACE)
add_library(Engine::warnings ALIAS engine_warnings)
engine_apply_warning_profile(engine_warnings)

engine_run_compiler_smoke_test()

add_subdirectory(engine)

if(ENGINE_BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

if(ENGINE_BUILD_TESTS)
  include(CTest)
  add_subdirectory(tests)
endif()

install(
  TARGETS engine_build_options engine_warnings
  EXPORT EngineTargets
)

install(
  EXPORT EngineTargets
  NAMESPACE Engine::
  FILE EngineTargets.cmake
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Engine"
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/EngineConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/EngineConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Engine"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/EngineConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/EngineConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/EngineConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Engine"
)
