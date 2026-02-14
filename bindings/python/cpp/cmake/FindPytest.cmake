find_program(PYTEST_EXECUTABLE pytest REQUIRED)

execute_process(
    COMMAND ${PYTEST_EXECUTABLE} --version
    OUTPUT_VARIABLE _pytest_version
    ERROR_QUIET
)

string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" Pytest_VERSION "${_pytest_version}")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Pytest
    REQUIRED_VARS PYTEST_EXECUTABLE
    VERSION_VAR Pytest_VERSION
)
