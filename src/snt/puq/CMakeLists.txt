# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-puq)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
target_link_libraries(${EXEC_NAME} PRIVATE snt-core snt-exs snt-val)

target_include_directories(${EXEC_NAME}
  PUBLIC	
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
)

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
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/snt/puq/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snt/puq
    FILES_MATCHING PATTERN "*.h"
)
