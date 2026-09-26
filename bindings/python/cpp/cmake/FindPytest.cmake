if(NOT Python3_EXECUTABLE)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)
endif()

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -m pytest --version
    RESULT_VARIABLE _pytest_result
    OUTPUT_VARIABLE _pytest_version
    ERROR_VARIABLE _pytest_error
)

if(_pytest_result EQUAL 0)
    string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" Pytest_VERSION "${_pytest_version}")
    set(PYTEST_EXECUTABLE "${Python3_EXECUTABLE}")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    Pytest
    REQUIRED_VARS PYTEST_EXECUTABLE Pytest_VERSION
    VERSION_VAR Pytest_VERSION
    REASON_FAILURE_MESSAGE "Install pytest in ${Python3_EXECUTABLE} or select a Python interpreter that has it (-DPython3_EXECUTABLE=...). ${_pytest_error}"
)
