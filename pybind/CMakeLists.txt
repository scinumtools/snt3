find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})
find_package(pybind11 CONFIG)

set(MODULE_NAME scinumtools3)

file(GLOB SOURCE_FILES "./*.cpp")
pybind11_add_module(${MODULE_NAME} ${SOURCE_FILES})
target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
set_target_properties(${MODULE_NAME} PROPERTIES
    OUTPUT_NAME ${MODULE_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# register pytests
function(register_pytests MODULE_DIR)
file(GLOB_RECURSE PYTEST_FILES "${CMAKE_SOURCE_DIR}/pytest/${MODULE_DIR}/test_*.py")
foreach(TEST_FILE ${PYTEST_FILES})
    get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
    string(REGEX REPLACE "^test_" "" TEST_NAME ${TEST_NAME})
    set(TEST_FULL_NAME pytest.${MODULE_DIR}.${TEST_NAME})
    add_test(
        NAME ${TEST_FULL_NAME}
        COMMAND ${Python3_EXECUTABLE} -m pytest -v ${TEST_FILE}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/pytest
    )
    set_tests_properties(${TEST_FULL_NAME} PROPERTIES DEPENDS ${MODULE_NAME})
endforeach()
endfunction()

if (ENABLE_SNT_PYBIND)
   add_subdirectory(snt)
endif()

if (ENABLE_PUQ_PYBIND)
   add_subdirectory(puq)
endif()

if (ENABLE_DIP_PYBIND)
   add_subdirectory(dip)
endif()

enable_testing()

# install python module
install(TARGETS ${MODULE_NAME}
        LIBRARY DESTINATION ${Python3_SITEARCH})  # or ${Python3_SITELIB}

### register pytest into ctest
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
find_package(Pytest 8.2.1 REQUIRED)

