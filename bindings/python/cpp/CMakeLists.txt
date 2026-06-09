find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 CONFIG REQUIRED)

set(MODULE_NAME _core)

file(GLOB SOURCE_FILES "./*.cpp")
pybind11_add_module(${MODULE_NAME} ${SOURCE_FILES})

target_include_directories(${MODULE_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

set_target_properties(${MODULE_NAME} PROPERTIES
    OUTPUT_NAME ${MODULE_NAME}
    #LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/scinumtools3
)

file(MAKE_DIRECTORY
    ${CMAKE_BINARY_DIR}/python/scinumtools3
)

file(COPY
    ${CMAKE_SOURCE_DIR}/bindings/python/src/scinumtools3
    DESTINATION ${CMAKE_BINARY_DIR}/python
)

# this needs to be here, so that ctest notice it!
if(
    ENABLE_SNT_PYTEST OR
    ENABLE_EXS_PYTEST OR
    ENABLE_VAL_PYTEST OR
    ENABLE_PUQ_PYTEST OR
    ENABLE_DIP_PYTEST OR
    ENABLE_MAT_PYTEST OR
    ENABLE_API_PYTEST
)

    # register pytests
    function(register_pytests MODULE_DIR)
    file(GLOB_RECURSE PYTEST_FILES "${CMAKE_SOURCE_DIR}/bindings/python/tests/${MODULE_DIR}/test_*.py")
    foreach(TEST_FILE ${PYTEST_FILES})
        get_filename_component(TEST_NAME ${TEST_FILE} NAME_WE)
        string(REGEX REPLACE "^test_" "" TEST_NAME ${TEST_NAME})
        set(TEST_FULL_NAME pytest.${MODULE_DIR}.${TEST_NAME})
        add_test(
            NAME ${TEST_FULL_NAME}
            COMMAND ${Python3_EXECUTABLE} -m pytest -v ${TEST_FILE}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bindings/python/tests
        )
        set_tests_properties(${TEST_FULL_NAME} PROPERTIES DEPENDS ${MODULE_NAME})
    endforeach()
    endfunction()

endif()

if (ENABLE_SNT_PYBIND)
   add_subdirectory(snt)
endif()

if (ENABLE_PUQ_PYBIND)
   add_subdirectory(puq)
endif()

if (ENABLE_DIP_PYBIND)
   add_subdirectory(dip)
endif()

if (ENABLE_API_PYBIND)
   add_subdirectory(api)
endif()

if(
    ENABLE_SNT_PYTEST OR
    ENABLE_EXS_PYTEST OR
    ENABLE_VAL_PYTEST OR
    ENABLE_PUQ_PYTEST OR
    ENABLE_DIP_PYTEST OR
    ENABLE_MAT_PYTEST OR
    ENABLE_API_PYTEST
)
    register_pytests(misc)
    
    enable_testing()
    
    ### register pytest into ctest
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
    find_package(Pytest 8.2.1 REQUIRED)
endif()

# install python module
install(TARGETS ${MODULE_NAME}
        LIBRARY DESTINATION scinumtools3)

