cmake_minimum_required(VERSION 3.22)
project(dip_cmake_integration LANGUAGES CXX)

find_package(snt CONFIG REQUIRED)

snt_dip_get(FILE build.dip PATH build.tools TYPE BOOL OUTPUT BUILD_TOOLS)
snt_dip_get(FILE build.dip PATH build.standard TYPE INTEGER OUTPUT CXX_STANDARD)
snt_dip_get(FILE build.dip PATH build.message TYPE STRING OUTPUT BUILD_MESSAGE)

message(STATUS "${BUILD_MESSAGE}")
if(BUILD_TOOLS)
    add_executable(dip-cmake-example main.cpp)
    set_target_properties(dip-cmake-example PROPERTIES
        CXX_STANDARD "${CXX_STANDARD}"
        CXX_STANDARD_REQUIRED YES
    )
endif()
