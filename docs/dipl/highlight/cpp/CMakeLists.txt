cmake_minimum_required(VERSION 3.16)
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  project(snt_dipl_highlighter LANGUAGES CXX)
endif()

add_library(snt-dipl-highlighter STATIC dipl_highlighter.cpp)
add_library(snt::dipl_highlighter ALIAS snt-dipl-highlighter)
target_include_directories(snt-dipl-highlighter PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_features(snt-dipl-highlighter PUBLIC cxx_std_17)
