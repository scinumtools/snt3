# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME scinumtools-mat)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
target_compile_definitions(${EXEC_NAME} PRIVATE)
target_link_libraries(${EXEC_NAME} PRIVATE scinumtools-puq)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MODULE_NAME})
