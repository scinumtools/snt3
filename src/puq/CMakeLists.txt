# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-puq)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${EXEC_NAME} PRIVATE snt-exs snt-val)

set_target_properties(${EXEC_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MODULE_NAME})
