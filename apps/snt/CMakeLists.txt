# create executable
set(EXEC_NAME snt)

file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
if(NOT ENABLE_SNT_REPORT)
  list(FILTER EXEC_FILES EXCLUDE REGEX "/module_report\\.cpp$")
endif()
add_executable(${EXEC_NAME} ${EXEC_FILES} )
add_subdirectory(../snt-hub ${CMAKE_CURRENT_BINARY_DIR}/hub)
if(ENABLE_SNT_MCP)
  target_compile_definitions(snt PRIVATE ENABLE_SNT_MCP)
  add_subdirectory(../snt-mcp ${CMAKE_CURRENT_BINARY_DIR}/mcp)
endif()

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

# Generate shell completions from the CLI's own completion metadata.
set(SNT_COMPLETION_DIR "${CMAKE_CURRENT_BINARY_DIR}/completions")
add_custom_command(TARGET ${EXEC_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SNT_COMPLETION_DIR}"
  COMMAND ${CMAKE_COMMAND}
    "-DSNT_EXECUTABLE=$<TARGET_FILE:${EXEC_NAME}>"
    "-DOUTPUT_FILE=${SNT_COMPLETION_DIR}/snt"
    "-DSHELL=bash"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/write_completion.cmake"
  COMMAND ${CMAKE_COMMAND}
    "-DSNT_EXECUTABLE=$<TARGET_FILE:${EXEC_NAME}>"
    "-DOUTPUT_FILE=${SNT_COMPLETION_DIR}/_snt"
    "-DSHELL=zsh"
    -P "${CMAKE_CURRENT_SOURCE_DIR}/write_completion.cmake"
  VERBATIM)
install(FILES "${SNT_COMPLETION_DIR}/snt"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/bash-completion/completions)
install(FILES "${SNT_COMPLETION_DIR}/_snt"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/zsh/site-functions)

install(EXPORT SNTExecutables
        FILE snt-executables.cmake
        NAMESPACE snt::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/snt)

if(ENABLE_UNIT_TESTS)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)
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

  function(add_snt_pytest name filename)
    add_test(NAME ${name}
      COMMAND ${CMAKE_COMMAND} -E env
        "SNT_EXECUTABLE=$<TARGET_FILE:snt>"
        "SERVER_ENABLED=${ENABLE_SNT_SERVER}"
        "DMAP_ENABLED=${ENABLE_SNT_DMAP}"
        "VIEW_ENABLED=${ENABLE_SNT_VIEW}"
        "MCP_ENABLED=${ENABLE_SNT_MCP}"
        "REPORT_ENABLED=${ENABLE_SNT_REPORT}"
        "${Python3_EXECUTABLE}" -m pytest -q
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/${filename}")
  endfunction()

  if(ENABLE_SNT_REPORT)
    add_snt_pytest(cli.report test_report.py)
  endif()
  add_snt_pytest(cli.dip.persistence test_dip_persistence.py)
  add_snt_pytest(cli.init test_init.py)
  add_snt_pytest(cli.dip.comparison test_dip_comparison.py)
  add_snt_pytest(cli.dip.semantic test_dip_semantic.py)
  add_snt_pytest(cli.dip.bundle test_dip_bundle.py)
  add_snt_pytest(cli.dip.formatter test_formatter.py)
  add_snt_pytest(cli.dip.validator test_validator.py)
  add_snt_pytest(cli.dip.linter test_linter.py)
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
    add_snt_pytest(cli.dmap test_dmap.py)
  endif()
  add_snt_pytest(cli.commands test_commands.py)
endif()

enable_coverage(${EXEC_NAME})
