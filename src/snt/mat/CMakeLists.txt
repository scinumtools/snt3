# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-mat)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
target_compile_definitions(${EXEC_NAME} PRIVATE)
target_link_libraries(${EXEC_NAME} PRIVATE snt-puq)

set_target_properties(${EXEC_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# install libraries and executables
install(
    TARGETS ${EXEC_NAME}
    EXPORT TargetsSNT
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# install header files 
install(
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/snt/mat/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snt/mat
    FILES_MATCHING PATTERN "*.h"
)


enable_coverage(${EXEC_NAME})
