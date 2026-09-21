# create executable
set(EXEC_NAME snt-server)
set(CPPHTTPLIB_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-httplib/httplib.h")

if(NOT EXISTS "${CPPHTTPLIB_HEADER}")
  message(FATAL_ERROR
    "snt-server requires the cpp-httplib Git submodule. Run:\n"
    "  git submodule update --init --recursive"
  )
endif()

add_executable(${EXEC_NAME}
  dip.cpp
  main.cpp
  openapi.cpp
  published.cpp
  puq.cpp
  server.cpp
)

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    ${CMAKE_CURRENT_SOURCE_DIR}/external/cpp-httplib
)

target_link_libraries(${EXEC_NAME} PRIVATE snt-api)

enable_coverage(${EXEC_NAME})

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

if(ENABLE_UNIT_TESTS)
  add_subdirectory(tests)
endif()
