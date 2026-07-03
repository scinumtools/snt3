cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0074 NEW)
set(CMAKE_TRY_ENABLE_TARGET_TYPE STATIC_LIBRARY)

project(scinumtools VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # compile as UTF-8 on Windows
    add_compile_options(/utf-8)
else()
    # use flag only on macOS and linux
    add_compile_options(-Wno-deprecated)
endif()

set(MODULE_NAME snt)
#set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/install")
set(CMAKE_INSTALL_BINDIR bin)
set(CMAKE_INSTALL_LIBDIR lib)
set(CMAKE_INSTALL_INCLUDEDIR include)

# limit error number
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-ferror-limit=3)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fmax-errors=3)
endif()

# ensure all targets (executables, static libs, shared libs) are built with -fPIC if required
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# set environmental variables
file(READ "settings.env" ENV_CONTENTS)
string(REGEX MATCH "CODE_VERSION=([^\n\r]*)" _ ${ENV_CONTENTS})
string(REGEX REPLACE ".*CODE_VERSION=([^\n\r]*).*" "\\1" CODE_VERSION "${_}")

# set preprocessor flags
add_compile_definitions(CODE_VERSION="${CODE_VERSION}")
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/settings.h.in
    ${CMAKE_BINARY_DIR}/settings.h
)

# analyze code compilation
option(ENABLE_TIME_TRACE "Enable Clang time tracing" OFF)
if(ENABLE_TIME_TRACE)
    add_compile_options(-ftime-trace)
endif()

# pad strings right
function(pad_right out value width)
    string(LENGTH "${value}" len)
    math(EXPR pad "${width} - ${len}")

    if(pad GREATER 0)
        string(REPEAT " " ${pad} spaces)
        set(${out} "${value}${spaces}" PARENT_SCOPE)
    else()
        set(${out} "${value}" PARENT_SCOPE)
    endif()
endfunction()

# function that sets generic module options
function(set_module_options NAME DEFAULT_MODULES DEFAULT_PYTHON DEFAULT_TESTS)
    # set default values
    option(ENABLE_${NAME} "Enable ${NAME}" ${DEFAULT_MODULES})
    option(ENABLE_${NAME}_GTEST "Enable ${NAME} gtests" ${DEFAULT_TESTS})
    option(ENABLE_${NAME}_PYBIND "Enable ${NAME} pytests" ${DEFAULT_PYTHON})
    option(ENABLE_${NAME}_PYTEST "Enable ${NAME} pytests" ${DEFAULT_TESTS})
    # set parts off if main switch is off
    if(NOT ENABLE_${NAME})
        set(ENABLE_${NAME}_GTEST OFF CACHE BOOL "" FORCE)
        set(ENABLE_${NAME}_PYBIND OFF CACHE BOOL "" FORCE)
    endif()
    if(NOT ENABLE_${NAME}_PYBIND)
        set(ENABLE_${NAME}_PYTEST OFF CACHE BOOL "" FORCE)
    endif()
    pad_right( STATUS_MODULE "${ENABLE_${NAME}}" 7)
    pad_right( STATUS_GTEST "${ENABLE_${NAME}_GTEST}" 7)
    pad_right( STATUS_PYBIND "${ENABLE_${NAME}_PYBIND}" 7)
    pad_right( STATUS_PYTEST "${ENABLE_${NAME}_PYTEST}" 7)
    message(STATUS "${NAME}  ${STATUS_MODULE} ${STATUS_GTEST} ${STATUS_PYBIND} ${STATUS_PYTEST}")
endfunction()

# set options for all modules
message("Scientific Numerical Tools (SNT)")
message("Code version: ${CODE_VERSION}")
message(STATUS "Compiled parts:")
option(ENABLE_UNIT_TESTS "Main testing enabler" ON)  # switch off all tests
message(STATUS "ENABLE_UNIT_TESTS            ${ENABLE_UNIT_TESTS}")
option(ENABLE_BINDING_PYTHON "Main Python bind enabler" ON)  # switch off all pybinds 
message(STATUS "ENABLE_BINDING_PYTHON        ${ENABLE_BINDING_PYTHON}")
message(STATUS "     Module  GTests  PyBind  PyTests")
set_module_options(SNT ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(EXS ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(VAL ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(PUQ ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(DIP ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(MAT ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(API ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")

# enable building of executables
option(ENABLE_EXEC_APPS "Enable build of SNT application" ON)
message(STATUS "ENABLE_EXEC_APPS             ${ENABLE_EXEC_APPS}")
option(ENABLE_EXEC_APPS_SNT "Enable main SciNumTools CLI executable" "${ENABLE_EXEC_APPS}")
message(STATUS "ENABLE_EXEC_APPS_SNT         ${ENABLE_EXEC_APPS_SNT}")
option(ENABLE_EXEC_APPS_DMAP "Enable build of dimension map executable" "${ENABLE_EXEC_APPS}")
message(STATUS "ENABLE_EXEC_APPS_DMAP        ${ENABLE_EXEC_APPS_DMAP}")
option(ENABLE_EXEC_APPS_SERVER "Enable SNT REST-API server app" OFF)
message(STATUS "ENABLE_EXEC_APPS_SERVER      ${ENABLE_EXEC_APPS_SERVER}")
option(ENABLE_EXEC_APPS_GUI "Enable SNT graphical user interface" OFF)
message(STATUS "ENABLE_EXEC_APPS_GUI         ${ENABLE_EXEC_APPS_GUI}")
option(ENABLE_EXEC_EXAMPLES "Enable build of example executables" ON)
message(STATUS "ENABLE_EXEC_EXAMPLES         ${ENABLE_EXEC_EXAMPLES}")
option(ENABLE_EXEC_BENCHMARKS "Build benchmarks" OFF)
message(STATUS "ENABLE_EXEC_BENCHMARKS       ${ENABLE_EXEC_BENCHMARKS}")

# run ctests
option(RUN_UNIT_TESTS "Run tests after building" OFF)
message(STATUS "RUN_UNIT_TESTS               ${RUN_UNIT_TESTS}")

# import GTest
###############

# this needs to be here, so that ctest notice it!
if(ENABLE_UNIT_TESTS)
  find_package(GTest QUIET)  # usualy for Linux and macOS build
  if(NOT GTest_FOUND)  # needed for Windows build
      include(FetchContent)
      FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip
      )
      FetchContent_MakeAvailable(googletest)
  endif()
  include_directories(${GTEST_INCLUDE_DIRS})
  include(GoogleTest)
  enable_testing()
endif()

# enable clang-tidy during the compilation
##########################################

option(ENABLE_CLANG_TIDY "Run clang-tidy during build" OFF)
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_COMMAND NAMES clang-tidy)
    if(NOT CLANG_TIDY_COMMAND)
        message(FATAL_ERROR "clang-tidy not found!")
    else()
        message(STATUS "Running with clang-tidy ${ENABLE_CLANG_TIDY}")
    endif()

    # This makes all targets run clang-tidy automatically during compilation
    set(CMAKE_CXX_CLANG_TIDY ${CLANG_TIDY_COMMAND}
    			     -p=${CMAKE_BINARY_DIR})
endif()

# compile libraries and executables
###################################

add_subdirectory(src)                  # build dip-cpp library
if (ENABLE_EXEC_APPS)
    add_subdirectory(apps)             # build executable applications
endif()
if (ENABLE_EXEC_EXAMPLES)
    add_subdirectory(examples)         # build example applications
endif()
add_subdirectory(tests)                # build gtest executable
if (ENABLE_BINDING_PYTHON)         
   add_subdirectory(bindings/python/cpp)  # build Python bindings
endif()

# setup Google Benchmark
########################

if(ENABLE_EXEC_BENCHMARKS)

  include(FetchContent)
  
  FetchContent_Declare(
      benchmark
      GIT_REPOSITORY https://github.com/google/benchmark.git
      GIT_TAG v1.8.3
  )
  
  set(BENCHMARK_ENABLE_TESTING OFF)

  FetchContent_MakeAvailable(benchmark)

  set_target_properties(
      benchmark
      benchmark_main
      PROPERTIES
      CXX_CLANG_TIDY ""
  )

  add_subdirectory(benchmarks)
endif()

# create cmake export files
###########################

include(CMakePackageConfigHelpers)

configure_package_config_file(
	${CMAKE_CURRENT_SOURCE_DIR}/cmake/${MODULE_NAME}-config.cmake.in
  	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
  	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
	PATH_VARS CMAKE_INSTALL_INCLUDEDIR
)

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
    	VERSION ${CODE_VERSION}
    	COMPATIBILITY AnyNewerVersion
)

# install files
###############
	
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
)

install(EXPORT TargetsSNT
        FILE ${MODULE_NAME}-targets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}"
)

# running ctest
###############

if(RUN_UNIT_TESTS)
    execute_process(
        COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        RESULT_VARIABLE CTEST_RESULT
    )

    if(NOT CTEST_RESULT EQUAL 0)
        message(FATAL_ERROR "Unit tests failed.")
    endif()
endif()
