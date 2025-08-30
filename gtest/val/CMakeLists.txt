# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME gtest-val)

add_executable(${EXEC_NAME} ${TEST_FILES})
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main snt-val)
target_precompile_headers(${EXEC_NAME} PRIVATE pch_tests.h)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME} TEST_PREFIX gtest.val.)
gtest_discover_tests(${EXEC_NAME}
  TEST_PREFIX gtest.val.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
