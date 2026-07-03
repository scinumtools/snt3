find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
find_package(pybind11 QUIET)  # for Linux and macOS build
if(NOT pybind11_FOUND)        # for Windows build
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v3.0.1
    )
    FetchContent_MakeAvailable(pybind11)
endif()

set(MODULE_NAME _core)

file(GLOB SOURCE_FILES "./*.cpp")
pybind11_add_module(${MODULE_NAME} ${SOURCE_FILES})

# Link the Python import library so the linker can resolve Python C API symbols on Windows.
# Prefer the managed CMake targets provided by FindPython / FindPython3.
if (TARGET Python3::Python)
    message(STATUS "Linking ${MODULE_NAME} against Python3::Python")
    target_link_libraries(${MODULE_NAME} PRIVATE Python3::Python)
elseif (TARGET Python::Python)
    message(STATUS "Linking ${MODULE_NAME} against Python::Python")
    target_link_libraries(${MODULE_NAME} PRIVATE Python::Python)
else()
    # Diagnostic info if CI can't find the dev library
    message(STATUS "Python library target not found; printing Python3 variables for debugging:")
    message(STATUS "  Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
    message(STATUS "  Python3_VERSION: ${Python3_VERSION}")
    message(STATUS "  Python3_INCLUDE_DIRS: ${Python3_INCLUDE_DIRS}")
    message(STATUS "  Python3_LIBRARIES: ${Python3_LIBRARIES}")
endif()

target_include_directories(${MODULE_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

set_target_properties(${MODULE_NAME} PROPERTIES
    OUTPUT_NAME ${MODULE_NAME}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/scinumtools3    
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python/scinumtools3
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


if (ENABLE_VAL_PYBIND)
   add_subdirectory(val)
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

