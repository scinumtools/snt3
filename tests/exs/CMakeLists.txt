# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME gtest-exs)

add_executable(${EXEC_NAME} ${TEST_FILES})

# Here we have to define correct build paths for the examples
# Using Make/Ninja uses 'bin' directory, but Visual Studio uses 'bin/Release', 'bin/Debug', ...
target_compile_definitions(${EXEC_NAME} PRIVATE
    EXAMPLE_SOLVER_UNIQUE_PTR="$<TARGET_FILE:ExampleSolverUniquePtr>"
    EXAMPLE_SOLVER_CUSTOM="$<TARGET_FILE:ExampleSolverCustom>"
    EXAMPLE_SOLVER_DEFAULT="$<TARGET_FILE:ExampleSolverDefault>"
    EXAMPLE_SOLVER_ARRAY="$<TARGET_FILE:ExampleSolverArray>"
    EXAMPLE_SOLVER_SETTINGS="$<TARGET_FILE:ExampleSolverSettings>"
)

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main snt-exs)

if(NOT ENABLE_CLANG_TIDY)
target_precompile_headers(${EXEC_NAME} PRIVATE pch_tests.h)
endif()

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME}
  TEST_PREFIX gtest.exs.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

enable_coverage(${EXEC_NAME})
