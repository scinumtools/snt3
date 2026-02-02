function(pytest_discover_tests target)
    cmake_parse_arguments(
        PYTEST
        ""
        "WORKING_DIRECTORY;TRIM_FROM_NAME"
        "TEST_PATHS;LIBRARY_PATH_PREPEND;PYTHON_PATH_PREPEND;DEPENDS"
        ${ARGN}
    )

    add_test(
        NAME pytest_${target}
        COMMAND ${PYTEST_EXECUTABLE}
                ${PYTEST_TEST_PATHS}
        WORKING_DIRECTORY ${PYTEST_WORKING_DIRECTORY}
    )

    if(PYTEST_DEPENDS)
        set_tests_properties(pytest_${target}
            PROPERTIES DEPENDS "${PYTEST_DEPENDS}"
        )
    endif()
endfunction()