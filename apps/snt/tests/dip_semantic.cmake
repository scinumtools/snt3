set(INPUT "${SOURCE_DIR}/tests/dip/fixtures/generated_parameters.dip")
set(GRAPH_INPUT "${SOURCE_DIR}/examples/dip/SemanticDescription/parameters.dip")
file(READ "${CMAKE_CURRENT_LIST_DIR}/dip_semantic_expected.json" expected_template)
string(CONFIGURE "${expected_template}" expected_contract @ONLY)

function(check_json_output label actual expected)
  string(JSON equal ERROR_VARIABLE error EQUAL "${actual}" "${expected}")
  if(error OR NOT equal)
    message(FATAL_ERROR "${label} output changed:\nActual: ${actual}\nExpected: ${expected}\n${error}")
  endif()
endfunction()

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip describe --input "${INPUT}" --path experiment.steps --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "describe failed: ${error}")
endif()
string(JSON expected GET "${expected_contract}" describe)
check_json_output("describe" "${output}" "${expected}")

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip describe --input "${GRAPH_INPUT}" --path simulation.speed --record-graph --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "recorded describe failed: ${error}")
endif()
string(JSON expected GET "${expected_contract}" graph_describe)
check_json_output("recorded describe" "${output}" "${expected}")

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip list --input "${INPUT}" --query "?experiment." --limit 1 --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "list failed: ${error}")
endif()
string(JSON item GET "${expected_contract}" list_item)
set(expected "{\"schema_version\":\"1\",\"total\":4,\"truncated\":true,\"items\":[${item}]}")
check_json_output("list" "${output}" "${expected}")

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip preview --input "${INPUT}" --override "experiment.steps = 200" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "valid preview failed: ${error}")
endif()
string(JSON expected GET "${expected_contract}" valid_preview)
check_json_output("valid preview" "${output}" "${expected}")

execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip preview --input "${INPUT}" --override "missing = 2" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "invalid preview did not return JSON: ${error}")
endif()
string(JSON expected GET "${expected_contract}" invalid_preview)
check_json_output("invalid preview" "${output}" "${expected}")
