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

# enable apps
option(ENABLE_APP_SNT_SERVER "Enable SNT REST-API server app" OFF)
option(ENABLE_APP_SNT_GUI "Enable SNT graphical user interface" OFF)

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

# set generic options
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
message(STATUS "     Module  GTests  PyBind  PyTests")
option(ENABLE_UNIT_TESTS "Main testing enabler" ON)  # switch off all tests
option(ENABLE_BINDING_PYTHON "Main Python bind enabler" ON)  # switch off all pybinds 
set_module_options(SNT ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(EXS ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(VAL ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(PUQ ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(DIP ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(MAT ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")
set_module_options(API ON "${ENABLE_BINDING_PYTHON}" "${ENABLE_UNIT_TESTS}")

# import GTest
###############

# this needs to be here, so that ctest notice it!
if(
    ENABLE_SNT_GTEST OR
    ENABLE_EXS_GTEST OR
    ENABLE_VAL_GTEST OR
    ENABLE_PUQ_GTEST OR
    ENABLE_DIP_GTEST OR
    ENABLE_MAT_GTEST
)
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
add_subdirectory(apps)                 # build executable applications
add_subdirectory(examples)             # build example applications
add_subdirectory(tests)                # build gtest executable
if (ENABLE_PUQ AND ENABLE_DIP)         
   add_subdirectory(bindings/python/cpp)  # build Python bindings
endif()

# setup Google Benchmark
########################

option(ENABLE_BENCHMARKS "Build benchmarks" OFF)

if(ENABLE_BENCHMARKS)

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
	
install(DIRECTORY include/
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
	FILES_MATCHING PATTERN "*.h")
	
install(FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config.cmake"
	"${CMAKE_CURRENT_BINARY_DIR}/${MODULE_NAME}-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}
)

install(EXPORT TargetsSNT
        FILE ${MODULE_NAME}-targets.cmake
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${MODULE_NAME}"
)

