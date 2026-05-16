SET(EXEC_NAME benchmark-puq)

add_executable(
    ${EXEC_NAME}
    puq/puq_benchmark.cpp
)

target_link_libraries(
    ${EXEC_NAME}
    PRIVATE
    benchmark::benchmark
    snt-puq
)

target_compile_features(
    ${EXEC_NAME}
    PRIVATE
    cxx_std_20
)

set_target_properties(
    ${EXEC_NAME}
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin	
    CXX_CLANG_TIDY ""
)
