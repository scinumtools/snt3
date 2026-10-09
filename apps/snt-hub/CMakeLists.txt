find_package(CURL REQUIRED)
target_sources(snt PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/hub.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/process.cpp")
target_include_directories(snt PRIVATE "${PROJECT_SOURCE_DIR}/external")
target_link_libraries(snt PRIVATE CURL::libcurl)

if(ENABLE_UNIT_TESTS)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(NAME cli.hub
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_cli.py"
      "$<TARGET_FILE:snt>")
endif()
