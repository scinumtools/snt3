# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME GTestModule-puq)

add_executable(${EXEC_NAME} ${TEST_FILES})
target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main scinumtools-puq)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

gtest_discover_tests(${EXEC_NAME}
  TEST_PREFIX gtest.puq.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
