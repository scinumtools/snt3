# build library
find_path(SNT_BRIEFPP_INCLUDE_DIR briefpp/report.hpp
  HINTS "${PROJECT_SOURCE_DIR}/external/briefpp/include"
  DOC "Directory containing briefpp/report.hpp")
if(NOT SNT_BRIEFPP_INCLUDE_DIR)
  message(FATAL_ERROR
    "DIP reports require Brief++. Initialize external/briefpp or set SNT_BRIEFPP_INCLUDE_DIR.")
endif()
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
    ${SNT_BRIEFPP_INCLUDE_DIR}
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
