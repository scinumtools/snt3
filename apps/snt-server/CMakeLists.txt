include("${PROJECT_SOURCE_DIR}/apps/http.cmake")

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
