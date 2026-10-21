include("${PROJECT_SOURCE_DIR}/apps/http.cmake")
target_sources(snt PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tools.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/protocol.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/storage.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/contracts.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/execution.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/transports.cpp")
target_include_directories(snt PRIVATE "${PROJECT_SOURCE_DIR}/external")

if(ENABLE_UNIT_TESTS)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
  add_test(NAME cli.mcp.help COMMAND snt mcp --help)
  set_tests_properties(cli.mcp.help PROPERTIES PASS_REGULAR_EXPRESSION "Model Context Protocol")
  add_test(NAME cli.mcp
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_mcp.py" "$<TARGET_FILE:snt>")
  set_tests_properties(cli.mcp PROPERTIES TIMEOUT 90)
endif()
