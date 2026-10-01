add_executable(ExampleDipInspectionGraph main.cpp)
target_link_libraries(ExampleDipInspectionGraph PRIVATE snt-dip)
set_target_properties(ExampleDipInspectionGraph PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
enable_coverage(ExampleDipInspectionGraph)
