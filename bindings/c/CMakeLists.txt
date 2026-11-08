add_library(snt-c STATIC src/puq.cpp src/dip.cpp src/dip_nodes.cpp src/comparison.cpp)
target_include_directories(snt-c PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>)
target_link_libraries(snt-c PUBLIC snt-puq snt-dip)
install(TARGETS snt-c EXPORT TargetsSNT ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Rust links these static archives with a C linker. Record the C++ runtime
# selected by CMake so the Rust build can link the matching one on Linux.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    if("c++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
        set(SNT_CXX_RUNTIME "c++")
    elseif("stdc++" IN_LIST CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
        set(SNT_CXX_RUNTIME "stdc++")
    else()
        message(FATAL_ERROR "Could not determine the C++ runtime for snt-c")
    endif()
    file(WRITE "${CMAKE_BINARY_DIR}/snt-cxx-runtime.txt" "${SNT_CXX_RUNTIME}\n")
    foreach(SNT_CXX_RUNTIME_DIR IN LISTS CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES)
        file(APPEND "${CMAKE_BINARY_DIR}/snt-cxx-runtime.txt"
             "${SNT_CXX_RUNTIME_DIR}\n")
    endforeach()
    install(FILES "${CMAKE_BINARY_DIR}/snt-cxx-runtime.txt"
            DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/snt")
endif()

if(ENABLE_CABI_GTEST)
    add_subdirectory(tests)
endif()
