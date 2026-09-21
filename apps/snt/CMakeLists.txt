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
  add_test(
    NAME cli.dip.persistence
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/dip-persistence"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dip_persistence.cmake"
  )
endif()

enable_coverage(${EXEC_NAME})
