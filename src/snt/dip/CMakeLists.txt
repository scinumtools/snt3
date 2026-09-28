# build library
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/report/template.tex")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/report/template.tex" SNT_REPORT_TEMPLATE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/report/template.h.in"
               "${CMAKE_CURRENT_BINARY_DIR}/report/template.h" @ONLY)
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-dip)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
# snt-dip is static.  HDF5 must therefore propagate to every final binary
# that consumes it, including the Python extension built for wheels.
target_link_libraries(${EXEC_NAME} PUBLIC snt-puq HDF5::HDF5)

target_include_directories(${EXEC_NAME}
  PUBLIC	
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/report
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
    DIRECTORY ${PROJECT_SOURCE_DIR}/include/snt/dip/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snt/dip
    FILES_MATCHING PATTERN "*.h"
)


enable_coverage(${EXEC_NAME})
