include(FetchContent)

function(engine_probe_url_access url out_result)
  set(probe_file "${CMAKE_BINARY_DIR}/engine_url_probe.tmp")
  file(DOWNLOAD "${url}" "${probe_file}" TIMEOUT 5 STATUS probe_status LOG probe_log)
  list(GET probe_status 0 probe_code)
  if(probe_code EQUAL 0)
    set(${out_result} TRUE PARENT_SCOPE)
  else()
    set(${out_result} FALSE PARENT_SCOPE)
  endif()
endfunction()

function(engine_resolve_sdl2 out_target)
  set(resolved_target "")

  find_package(SDL2 QUIET)

  if(TARGET SDL2::SDL2)
    set(resolved_target "SDL2::SDL2")
  elseif(TARGET SDL2::SDL2-static)
    set(resolved_target "SDL2::SDL2-static")
  elseif(TARGET SDL2-static)
    set(resolved_target "SDL2-static")
  endif()

  if(NOT resolved_target AND ENGINE_AUTO_FETCH_SDL2)
    engine_probe_url_access("https://github.com" ENGINE_CAN_ACCESS_GITHUB)
    if(ENGINE_CAN_ACCESS_GITHUB)
      message(STATUS "SDL2 not found locally, fetching from source")

      set(SDL_SHARED OFF CACHE BOOL "" FORCE)
      set(SDL_STATIC ON CACHE BOOL "" FORCE)
      set(SDL_TEST OFF CACHE BOOL "" FORCE)

      FetchContent_Declare(
        sdl2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG release-2.30.7
        GIT_SHALLOW TRUE
      )
      FetchContent_MakeAvailable(sdl2)

      if(TARGET SDL2::SDL2)
        set(resolved_target "SDL2::SDL2")
      elseif(TARGET SDL2::SDL2-static)
        set(resolved_target "SDL2::SDL2-static")
      elseif(TARGET SDL2-static)
        set(resolved_target "SDL2-static")
      endif()

      # Any SDL2 targets we just fetched and are building must be part of our export set
      # because our exported engine targets depend on them.
      if(TARGET SDL2-static)
        install(TARGETS SDL2-static EXPORT EngineTargets)
      endif()
      if(TARGET SDL2)
        install(TARGETS SDL2 EXPORT EngineTargets)
      endif()
    else()
      message(STATUS "SDL2 auto-fetch requested but github.com is unreachable; continuing with SDL stub backend")
    endif()
  endif()

  set(${out_target} "${resolved_target}" PARENT_SCOPE)
endfunction()

function(engine_resolve_imgui out_target)
  set(resolved_target "")

  if(TARGET imgui::imgui)
    set(resolved_target "imgui::imgui")
  endif()

  if(NOT resolved_target AND ENGINE_AUTO_FETCH_IMGUI)
    engine_probe_url_access("https://github.com" ENGINE_CAN_ACCESS_GITHUB)
    if(ENGINE_CAN_ACCESS_GITHUB)
      message(STATUS "ImGui not found locally, fetching from source")

      FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.91.9b
        GIT_SHALLOW TRUE
      )
      FetchContent_MakeAvailable(imgui)

      add_library(engine_thirdparty_imgui STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
      )
      add_library(imgui::imgui ALIAS engine_thirdparty_imgui)

      target_include_directories(
        engine_thirdparty_imgui
        PUBLIC
          $<BUILD_INTERFACE:${imgui_SOURCE_DIR}>
          $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
      )

      install(TARGETS engine_thirdparty_imgui EXPORT EngineTargets)

      set(resolved_target "imgui::imgui")
    else()
      message(STATUS "ImGui auto-fetch requested but github.com is unreachable; continuing without ImGui linkage")
    endif()
  endif()

  set(${out_target} "${resolved_target}" PARENT_SCOPE)
endfunction()
