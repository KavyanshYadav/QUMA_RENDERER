function(engine_run_compiler_smoke_test)
  set(smoke_dir "${CMAKE_BINARY_DIR}/compiler_smoke")
  file(MAKE_DIRECTORY "${smoke_dir}")

  set(smoke_source "${smoke_dir}/compiler_smoke.cpp")
  file(WRITE "${smoke_source}" "#include <iostream>\nint main(){ std::cout << \"engine compiler smoke ok\"; return 0; }\n")

  try_compile(
    ENGINE_COMPILER_SMOKE_OK
    "${smoke_dir}/build"
    "${smoke_source}"
    CMAKE_FLAGS "-DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}"
    OUTPUT_VARIABLE ENGINE_COMPILER_SMOKE_LOG
    
    
  )
  if(NOT ENGINE_COMPILER_SMOKE_OK)
    message(FATAL_ERROR "Compiler smoke test failed:\n${ENGINE_COMPILER_SMOKE_LOG}")
  endif()

  message(STATUS "Compiler smoke test passed for ${CMAKE_CXX_COMPILER_ID} (${CMAKE_CXX_COMPILER})")
endfunction()
