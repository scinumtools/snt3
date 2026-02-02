#find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
#include_directories(${Python3_INCLUDE_DIRS})
#find_package(pybind11 CONFIG)
#
#set(MODULE_NAME scinumtools3)
#
#pybind11_add_module(${MODULE_NAME})
#target_include_directories(${MODULE_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
#enable_testing()
#
#add_subdirectory(snt)
##if (COMPILE_GTESTS_PUQ)	
##add_subdirectory(puq)
##endif()
##if (COMPILE_GTESTS_DIP)
##add_subdirectory(dip)
##endif()
#
#set_target_properties(${MODULE_NAME} PROPERTIES
#    OUTPUT_NAME ${MODULE_NAME}
#    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/python
#)
#
## install python module
#install(TARGETS ${MODULE_NAME}
#        LIBRARY DESTINATION ${Python3_SITEARCH})  # or ${Python3_SITELIB}
#
### register pytest into ctest
#list(APPEND CMAKE_MODULE_PATH
#     ${CMAKE_CURRENT_LIST_DIR}/cmake)
#include(Pytest)
#find_package(Pytest 8.2.1 REQUIRED)
#
#pytest_discover_tests(
#    ${MODULE_NAME}
#    TEST_PATHS ${CMAKE_SOURCE_DIR}/pytest
#    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/pytest
#    LIBRARY_PATH_PREPEND
#        $<TARGET_FILE_DIR:${MODULE_NAME}>
#    PYTHON_PATH_PREPEND
#        $<TARGET_FILE_DIR:${MODULE_NAME}>
#    TRIM_FROM_NAME "^test_"
#)
#
