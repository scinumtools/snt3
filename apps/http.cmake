# Shared HTTP dependency setup for the REST and MCP application frontends.
include_guard(GLOBAL)
if(SNT_HTTPLIB_INCLUDE_DIR AND NOT EXISTS "${SNT_HTTPLIB_INCLUDE_DIR}/httplib.h")
  unset(SNT_HTTPLIB_INCLUDE_DIR CACHE)
endif()
find_path(SNT_HTTPLIB_INCLUDE_DIR httplib.h
  HINTS "${PROJECT_SOURCE_DIR}/external/cpp-httplib"
  DOC "Directory containing cpp-httplib's httplib.h")
if(NOT SNT_HTTPLIB_INCLUDE_DIR)
  message(FATAL_ERROR
    "SNT HTTP applications require cpp-httplib. Install cpp-httplib, set SNT_HTTPLIB_INCLUDE_DIR, "
    "or run git submodule update --init --recursive.")
endif()
file(STRINGS "${SNT_HTTPLIB_INCLUDE_DIR}/httplib.h" httplib_version_line
  REGEX "^#define CPPHTTPLIB_VERSION ")
string(REGEX MATCH "[0-9]+\\.[0-9]+\\.[0-9]+" httplib_version "${httplib_version_line}")
if(NOT httplib_version OR httplib_version VERSION_LESS "0.46.0")
  message(FATAL_ERROR "SNT HTTP applications require cpp-httplib 0.46.0 or newer.")
endif()

find_package(Threads REQUIRED)
target_link_libraries(snt PRIVATE Threads::Threads)
if(WIN32)
  target_link_libraries(snt PRIVATE ws2_32)
endif()

target_include_directories(snt PRIVATE "${SNT_HTTPLIB_INCLUDE_DIR}")
target_sources(snt PRIVATE "${PROJECT_SOURCE_DIR}/apps/http_security.cpp")
