add_library(engine_platform_backend_sdl STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/src/SdlBackendStub.cpp
)
add_library(Engine::platform_backend_sdl ALIAS engine_platform_backend_sdl)

target_link_libraries(engine_platform_backend_sdl PUBLIC engine_platform_contract)

target_include_directories(
  engine_platform_backend_sdl
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

engine_resolve_sdl2(ENGINE_SDL2_TARGET)

if(ENGINE_SDL2_TARGET)
  target_sources(engine_platform_backend_sdl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/SdlPlatformBackend.cpp)
  target_link_libraries(engine_platform_backend_sdl PRIVATE $<BUILD_INTERFACE:${ENGINE_SDL2_TARGET}>)
  set(ENGINE_PLATFORM_HAS_SDL ON PARENT_SCOPE)
else()
  message(STATUS "SDL2 unavailable: engine_platform_backend_sdl will be a stub target")
endif()

install(TARGETS engine_platform_backend_sdl EXPORT EngineTargets)
