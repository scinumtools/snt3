# create executable
file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME SolverCustom)

add_executable(${EXEC_NAME} ${EXEC_FILES} )
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
