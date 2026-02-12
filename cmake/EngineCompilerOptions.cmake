function(engine_apply_warning_profile target)
  if(MSVC)
    target_compile_options(${target} INTERFACE /W4 /permissive- /wd4251)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(
      ${target}
      INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wnon-virtual-dtor
    )
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(
      ${target}
      INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wshadow
        -Wdocumentation
    )
  else()
    message(STATUS "No compiler warning profile configured for ${CMAKE_CXX_COMPILER_ID}")
  endif()
endfunction()
