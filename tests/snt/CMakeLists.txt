# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME gtest-snt)

add_executable(${EXEC_NAME} ${TEST_FILES})

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME}
  TEST_PREFIX gtest.snt.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
