# create executable
file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME ExampleSolverDefault)

add_executable(${EXEC_NAME} ${EXEC_FILES} )

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_link_libraries(${EXEC_NAME} PRIVATE snt-exs)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
