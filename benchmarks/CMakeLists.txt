function(add_puq_benchmark target source)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE benchmark::benchmark snt-puq)
    target_compile_features(${target} PRIVATE cxx_std_20)
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        CXX_CLANG_TIDY ""
    )
endfunction()

add_puq_benchmark(benchmark-puq puq/puq_benchmark.cpp)
add_puq_benchmark(benchmark-puq-scalar puq/scalar_benchmark.cpp)
add_puq_benchmark(benchmark-puq-array puq/array_benchmark.cpp)
