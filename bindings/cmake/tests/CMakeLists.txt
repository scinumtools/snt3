add_test(NAME cmake.dipl
    COMMAND "${CMAKE_COMMAND}"
        "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
        "-DSNT_HELPER=${PROJECT_SOURCE_DIR}/bindings/cmake/SNTDIPL.cmake"
        "-DTEST_ROOT=${CMAKE_CURRENT_BINARY_DIR}/consumer test"
        "-DTEST_GENERATOR=${CMAKE_GENERATOR}"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/integration.cmake"
)
