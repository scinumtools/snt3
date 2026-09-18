# build test
file(GLOB TEST_FILES "*.cpp")
set(EXEC_NAME gtest-api)

add_executable(${EXEC_NAME} ${TEST_FILES})

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

target_link_libraries(${EXEC_NAME} PRIVATE GTest::gtest_main snt-api)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

gtest_discover_tests(${EXEC_NAME}
  DISCOVERY_TIMEOUT 30
  TEST_PREFIX gtest.api.
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

enable_coverage(${EXEC_NAME})

if(TARGET snt)
  add_test(
    NAME cli.dip.persistence
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/dip-persistence"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/dip_persistence.cmake"
  )
endif()
