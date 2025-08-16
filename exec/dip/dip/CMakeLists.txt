# create executable
file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME dip)

add_executable(${EXEC_NAME} ${EXEC_FILES} )
target_include_directories(${EXEC_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(${EXEC_NAME} PRIVATE snt-dip)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
