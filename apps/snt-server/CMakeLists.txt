# Add the optional server module to the main executable.
find_path(SNT_HTTPLIB_INCLUDE_DIR httplib.h
  HINTS "${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-httplib"
  DOC "Directory containing cpp-httplib's httplib.h")
if(NOT SNT_HTTPLIB_INCLUDE_DIR)
  message(FATAL_ERROR
    "snt server requires cpp-httplib. Install cpp-httplib, set SNT_HTTPLIB_INCLUDE_DIR, "
    "or run git submodule update --init --recursive.")
endif()
file(STRINGS "${SNT_HTTPLIB_INCLUDE_DIR}/httplib.h" httplib_version_line
  REGEX "^#define CPPHTTPLIB_VERSION ")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" httplib_version "${httplib_version_line}")
if(NOT httplib_version OR httplib_version VERSION_LESS "0.46.0")
  message(FATAL_ERROR "snt server requires cpp-httplib 0.46.0 or newer.")
endif()

find_package(Threads REQUIRED)
target_link_libraries(snt PRIVATE Threads::Threads)
if(WIN32)
  target_link_libraries(snt PRIVATE ws2_32)
endif()

target_sources(snt PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/dip.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/openapi.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/published.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/puq.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp
)

target_include_directories(snt
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    ${SNT_HTTPLIB_INCLUDE_DIR}
)

if(ENABLE_UNIT_TESTS)
  add_subdirectory(tests)
endif()
