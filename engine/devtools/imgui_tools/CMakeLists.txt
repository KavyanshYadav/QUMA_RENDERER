add_library(engine_devtools_imgui_tools)
add_library(Engine::devtools_imgui_tools ALIAS engine_devtools_imgui_tools)

target_link_libraries(
  engine_devtools_imgui_tools
  PUBLIC
    Engine::devtools_contract
)

if(ENGINE_ENABLE_IMGUI)
  engine_resolve_imgui(ENGINE_IMGUI_TARGET)
  if(ENGINE_IMGUI_TARGET)
    target_link_libraries(engine_devtools_imgui_tools PUBLIC ${ENGINE_IMGUI_TARGET})
  else()
    message(STATUS "ImGui unavailable: building imgui_tools without ImGui linkage")
  endif()
endif()

target_include_directories(
  engine_devtools_imgui_tools
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_sources(
  engine_devtools_imgui_tools
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ImguiToolsSuite.cpp
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/devtools/imgui_tools/ImguiToolsSuite.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/engine/devtools/imgui_tools/ToolServices.hpp
)

install(TARGETS engine_devtools_imgui_tools EXPORT EngineTargets FILE_SET HEADERS)
