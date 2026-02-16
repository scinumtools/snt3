# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME gtest-mat)

add_executable(${EXEC_NAME} ${TEST_FILES})

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main snt-mat)

if(NOT ENABLE_CLANG_TIDY)
target_precompile_headers(${EXEC_NAME} PRIVATE pch_tests.h)
endif()

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME}
  TEST_PREFIX gtest.mat.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
