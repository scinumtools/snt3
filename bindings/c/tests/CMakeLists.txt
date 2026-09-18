set(EXEC_NAME gtest-cabi)

add_executable(${EXEC_NAME}
    test_dip.cpp
    test_puq.cpp
)

target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main snt-c)

set_target_properties(${EXEC_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME}
    DISCOVERY_TIMEOUT 30
    TEST_PREFIX cabi.
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

enable_coverage(${EXEC_NAME})
