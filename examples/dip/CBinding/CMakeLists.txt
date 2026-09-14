cmake_minimum_required(VERSION 3.22)
project(dip_c_binding_example C)

find_package(snt CONFIG REQUIRED)
add_executable(dip-c-example main.c)
target_link_libraries(dip-c-example PRIVATE snt-c)
