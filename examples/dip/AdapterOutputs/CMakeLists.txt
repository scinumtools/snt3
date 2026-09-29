add_executable(ExampleDipAdapter main.cpp)
target_link_libraries(ExampleDipAdapter PRIVATE snt-dip)
set_target_properties(ExampleDipAdapter PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
enable_coverage(ExampleDipAdapter)
