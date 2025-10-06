find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})
find_package(pybind11 CONFIG)

set(MODULE_NAME scinumtools3)

if (COMPILE_PUQ AND COMPILE_DIP)
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
pybind11_add_module(${MODULE_NAME} ${SOURCE_FILES})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${MODULE_NAME} PRIVATE snt-dip)
set_target_properties(${MODULE_NAME} PROPERTIES
    OUTPUT_NAME ${MODULE_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
)


if (COMPILE_TESTS_PUQ)
# register pytest into ctest
find_package(Pytest 8.2.1 REQUIRED)
pytest_discover_tests(
    "pytest"
    TEST_PATHS ${CMAKE_SOURCE_DIR}/pytest
    INCLUDE_FILE_PATH
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/pytest
    LIBRARY_PATH_PREPEND
        $<TARGET_FILE_DIR:${MODULE_NAME}>
    PYTHON_PATH_PREPEND
        $<TARGET_FILE_DIR:${MODULE_NAME}>
    TRIM_FROM_NAME "^test_"
    DEPENDS ${MODULE_NAME}
)
endif()

endif()
