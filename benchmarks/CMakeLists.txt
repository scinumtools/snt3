function(add_snt_benchmark target source library language_standard)
    add_executable(${target} ${source})
    target_link_libraries(${target} PRIVATE benchmark::benchmark ${library})
    target_compile_features(${target} PRIVATE ${language_standard})
    set_target_properties(${target} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        CXX_CLANG_TIDY ""
    )
endfunction()

add_snt_benchmark(benchmark-puq puq/puq_benchmark.cpp snt-puq cxx_std_20)
add_snt_benchmark(benchmark-puq-scalar puq/scalar_benchmark.cpp snt-puq cxx_std_20)
add_snt_benchmark(benchmark-puq-array puq/array_benchmark.cpp snt-puq cxx_std_20)

if(ENABLE_DIP)
    add_snt_benchmark(benchmark-dip dip/dip_benchmark.cpp snt-dip cxx_std_17)
    add_snt_benchmark(benchmark-dip-schema-map dip/schema_map_benchmark.cpp snt-dip cxx_std_17)
endif()
