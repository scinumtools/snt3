# The generator writes relative to its working directory. Keep test output
# outside the repository's generated headers.
file(MAKE_DIRECTORY "${TEST_DIR}/src/snt/puq/systems/dmaps")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dmap -e
  WORKING_DIRECTORY "${TEST_DIR}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)
if(NOT status EQUAL 0 OR NOT output MATCHES "Generating empty dmap file")
  message(FATAL_ERROR "Dimension-map generation failed: ${status}\n${output}${error}")
endif()
file(GLOB maps "${TEST_DIR}/src/snt/puq/systems/dmaps/dmap_*.h")
list(LENGTH maps count)
if(count LESS 1)
  message(FATAL_ERROR "Dimension-map generation created no files")
endif()
foreach(map IN LISTS maps)
  file(READ "${map}" content)
  if(NOT content STREQUAL "// Empty file")
    message(FATAL_ERROR "Unexpected map content in ${map}")
  endif()
endforeach()

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dmap
  WORKING_DIRECTORY "${TEST_DIR}"
  RESULT_VARIABLE status
  OUTPUT_VARIABLE output
  ERROR_VARIABLE error)
if(NOT status EQUAL 0 OR NOT output MATCHES "Generating dmap file")
  message(FATAL_ERROR "Dimension-map calculation failed: ${status}\n${output}${error}")
endif()
foreach(map IN LISTS maps)
  file(READ "${map}" content)
  if(NOT content MATCHES "Unit system:" OR NOT content MATCHES "Code version:")
    message(FATAL_ERROR "Generated dimension map is incomplete: ${map}")
  endif()
endforeach()
