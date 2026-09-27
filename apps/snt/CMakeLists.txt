# create executable
set(EXEC_NAME snt)

file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
add_executable(${EXEC_NAME} ${EXEC_FILES} )

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(${EXEC_NAME} PRIVATE snt-api)
target_sources(${EXEC_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/apps/snt-docs/module.cpp")
target_link_libraries(${EXEC_NAME} PRIVATE snt-docs)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME}
        EXPORT SNTExecutables
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(EXPORT SNTExecutables
        FILE snt-executables.cmake
        NAMESPACE snt::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/snt)

if(ENABLE_UNIT_TESTS)
  add_test(NAME cli.docs
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/docs"
      -P "${PROJECT_SOURCE_DIR}/apps/snt-docs/tests/docs.cmake")
  add_test(
    NAME cli.dip.persistence
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/dip-persistence"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dip_persistence.cmake"
  )
endif()

if(ENABLE_SNT_SERVER)
  target_compile_definitions(snt PRIVATE ENABLE_SNT_SERVER)
  add_subdirectory(../snt-server ${CMAKE_CURRENT_BINARY_DIR}/server)
endif()
if(ENABLE_SNT_DMAP)
  target_compile_definitions(snt PRIVATE ENABLE_SNT_DMAP)
  add_subdirectory(../snt-dmap ${CMAKE_CURRENT_BINARY_DIR}/dmap)
endif()
if(ENABLE_SNT_VIEW)
  target_compile_definitions(snt PRIVATE ENABLE_SNT_VIEW)
  add_subdirectory(../snt-view ${CMAKE_CURRENT_BINARY_DIR}/view)
endif()


if(ENABLE_UNIT_TESTS)
  if(ENABLE_SNT_DMAP)
    add_test(NAME cli.dmap
      COMMAND ${CMAKE_COMMAND}
        "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
        "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/dmap"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dmap.cmake")
  endif()
  add_test(NAME cli.commands
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DSERVER_ENABLED=${ENABLE_SNT_SERVER}"
      "-DDMAP_ENABLED=${ENABLE_SNT_DMAP}"
      "-DVIEW_ENABLED=${ENABLE_SNT_VIEW}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/commands.cmake")
endif()

enable_coverage(${EXEC_NAME})
