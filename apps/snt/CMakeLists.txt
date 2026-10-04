# create executable
set(EXEC_NAME snt)

file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
if(NOT ENABLE_SNT_REPORT)
  list(FILTER EXEC_FILES EXCLUDE REGEX "/module_report\\.cpp$")
endif()
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
  set(GENERATED_PARAMETERS_DIR "${CMAKE_CURRENT_BINARY_DIR}/tests/generated-parameters")
  set(GENERATED_CPP_HEADER "${GENERATED_PARAMETERS_DIR}/parameters.hpp")
  set(GENERATED_C_HEADER "${GENERATED_PARAMETERS_DIR}/parameters.h")
  set(GENERATED_PARAMETERS_DIP "${PROJECT_SOURCE_DIR}/tests/dip/fixtures/generated_parameters.dip")
  add_custom_command(
    OUTPUT "${GENERATED_CPP_HEADER}" "${GENERATED_C_HEADER}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GENERATED_PARAMETERS_DIR}"
    COMMAND $<TARGET_FILE:snt> dip parse -i file
      "${GENERATED_PARAMETERS_DIP}"
      --generate cpp "${GENERATED_CPP_HEADER}"
    COMMAND $<TARGET_FILE:snt> dip parse -i file
      "${GENERATED_PARAMETERS_DIP}"
      --generate c "${GENERATED_C_HEADER}"
    DEPENDS snt "${GENERATED_PARAMETERS_DIP}"
    VERBATIM
  )
  add_custom_target(snt-generated-parameters
    DEPENDS "${GENERATED_CPP_HEADER}" "${GENERATED_C_HEADER}")
  add_executable(snt-generated-parameters-cpp
    "${PROJECT_SOURCE_DIR}/tests/dip/fixtures/generated_parameters_cpp.cpp")
  add_executable(snt-generated-parameters-c
    "${PROJECT_SOURCE_DIR}/tests/dip/fixtures/generated_parameters_c.c")
  target_include_directories(snt-generated-parameters-cpp PRIVATE "${GENERATED_PARAMETERS_DIR}")
  target_include_directories(snt-generated-parameters-c PRIVATE "${GENERATED_PARAMETERS_DIR}")
  add_dependencies(snt-generated-parameters-cpp snt-generated-parameters)
  add_dependencies(snt-generated-parameters-c snt-generated-parameters)
  add_test(NAME cli.generated.cpp-values COMMAND $<TARGET_FILE:snt-generated-parameters-cpp>)
  add_test(NAME cli.generated.c-values COMMAND $<TARGET_FILE:snt-generated-parameters-c>)

  if(ENABLE_SNT_REPORT)
    add_test(NAME cli.report
      COMMAND ${CMAKE_COMMAND}
        "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
        "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/report"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/report.cmake")
  endif()
  add_test(
    NAME cli.dip.persistence
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/dip-persistence"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dip_persistence.cmake"
  )
  add_test(
    NAME cli.dip.comparison
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests/dip-comparison"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/dip_comparison.cmake"
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
      "-DREPORT_ENABLED=${ENABLE_SNT_REPORT}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/commands.cmake")
endif()

enable_coverage(${EXEC_NAME})
