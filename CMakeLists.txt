cmake_minimum_required(VERSION 3.22)
cmake_policy(SET CMP0074 NEW)
set(CMAKE_TRY_ENABLE_TARGET_TYPE STATIC_LIBRARY)

project(scinumtools VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-deprecated -O3")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

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

# set generic options
function(set_module_options NAME DEFAULT)
    # set default values
    option(ENABLE_${NAME} "Enable ${NAME}" ${DEFAULT})
    option(ENABLE_${NAME}_GTEST "Enable ${NAME} gtests" ${DEFAULT})
    option(ENABLE_${NAME}_PYBIND "Enable ${NAME} pytests" ${DEFAULT})
    option(ENABLE_${NAME}_PYTEST "Enable ${NAME} pytests" ${DEFAULT})
    # set parts off if main switch is off
    if(NOT ENABLE_${NAME})
        set(ENABLE_${NAME}_GTEST OFF CACHE BOOL "" FORCE)
        set(ENABLE_${NAME}_PYBIND OFF CACHE BOOL "" FORCE)
    endif()
    if(NOT ENABLE_${NAME}_PYBIND)
        set(ENABLE_${NAME}_PYTEST OFF CACHE BOOL "" FORCE)
    endif()
endfunction()
set_module_options(SNT ON)
set_module_options(EXS ON)
set_module_options(VAL ON)
set_module_options(PUQ ON)
set_module_options(DIP ON)
set_module_options(MAT ON)

# set specialised options
set(ENABLE_PUQ_MAGNITUDE "value" CACHE STRING "Set the type of magnitude in PUQ module (array, value)")
if(ENABLE_PUQ_MAGNITUDE STREQUAL "array")
    add_compile_definitions(ENABLE_PUQ_MAGNITUDE_ARRAY)
elseif(ENABLE_PUQ_MAGNITUDE STREQUAL "value")
    add_compile_definitions(ENABLE_PUQ_MAGNITUDE_VALUE)
else()
    message(FATAL_ERROR "Invalid ENABLE_PUQ_MAGNITUDE: ${ENABLE_PUQ_MAGNITUDE}. Must be 'array' or 'value'.")
endif()

# import GTest
###############

# this needs to be here, so that ctest notice it!

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include(GoogleTest)
enable_testing()

message("Scientific Numerical Tools (SNT)")
message("Code version: ${CODE_VERSION}")

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
add_subdirectory(bindings/python/cpp)  # build Python bindings

# create include directory
##########################

file(GLOB_RECURSE PROJECT_HEADERS "${CMAKE_SOURCE_DIR}/src/*.h")

set(HEADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/include")

foreach(header ${PROJECT_HEADERS})
    # Strip "src/" prefix from path
    file(RELATIVE_PATH rel_path "${CMAKE_SOURCE_DIR}/src" "${header}")
    set(dest "${HEADER_OUTPUT_DIR}/${rel_path}")
    get_filename_component(dest_dir "${dest}" DIRECTORY)

    add_custom_command(
        OUTPUT "${dest}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${dest_dir}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${header}" "${dest}"
        DEPENDS "${header}"
        COMMENT "Copying ${header} -> ${dest}"
    )
    list(APPEND COPIED_HEADERS "${dest}")
endforeach()

add_custom_target(copy_headers ALL DEPENDS ${COPIED_HEADERS})

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
	
install(DIRECTORY src/
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

