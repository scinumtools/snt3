# build library
if(ENABLE_SNT_REPORT)
  find_path(SNT_BRIEFPP_INCLUDE_DIR briefpp/report.hpp
    HINTS "${PROJECT_SOURCE_DIR}/external/briefpp/include"
    DOC "Directory containing briefpp/report.hpp")
  if(NOT SNT_BRIEFPP_INCLUDE_DIR)
    message(FATAL_ERROR
      "DIP reports require Brief++. Initialize external/briefpp or set SNT_BRIEFPP_INCLUDE_DIR.")
  endif()
endif()
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
if(ENABLE_SNT_REPORT)
  list(FILTER SOURCE_FILES EXCLUDE REGEX "/report/disabled\\.cpp$")
else()
  list(FILTER SOURCE_FILES EXCLUDE REGEX "/report/(compiler|model|renderer|report)\\.cpp$")
endif()
set(EXEC_NAME snt-dip)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
# snt-dip is static.  HDF5 must therefore propagate to every final binary
# that consumes it, including the Python extension built for wheels.
target_link_libraries(${EXEC_NAME} PUBLIC snt-puq HDF5::HDF5)
# FindHDF5 can report "Included by compiler wrappers" without adding a C
# library to HDF5::HDF5 or HDF5_LIBRARIES.  That leaves every consumer of this
# static library with unresolved H5* symbols, including the DIP tests.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT HDF5_LIBRARIES)
  find_library(SNT_HDF5_C_LIBRARY NAMES hdf5
    PATH_SUFFIXES hdf5/serial serial REQUIRED)
  target_link_libraries(${EXEC_NAME} PUBLIC ${SNT_HDF5_C_LIBRARY})
endif()
if(ENABLE_SNT_REPORT)
  target_compile_definitions(${EXEC_NAME} PUBLIC SNT_ENABLE_REPORT)
endif()

target_include_directories(${EXEC_NAME}
  PUBLIC	
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    $<$<BOOL:${ENABLE_SNT_REPORT}>:${SNT_BRIEFPP_INCLUDE_DIR}>
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
