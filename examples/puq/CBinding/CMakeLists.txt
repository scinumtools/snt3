cmake_minimum_required(VERSION 3.22)
project(puq_c_binding_example LANGUAGES C CXX)

find_package(snt CONFIG REQUIRED)

add_executable(puq-c-example main.c)
target_link_libraries(puq-c-example PRIVATE snt-c)
target_include_directories(puq-c-example PRIVATE ${SNT_INCLUDE_DIR})
set_target_properties(puq-c-example PROPERTIES LINKER_LANGUAGE CXX)
