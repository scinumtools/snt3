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
  COMMAND "${SNT_EXECUTABLE}" dip override-contract --input "${INPUT}" --path experiment.steps --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "contract failed: ${error}")
endif()
string(JSON expected GET "${expected_contract}" contract)
check_json_output("contract" "${output}" "${expected}")

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

set(SCHEMA_INPUT "${SOURCE_DIR}/tests/dip/fixtures/schema_hierarchy.dip")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip schemas --input "${SCHEMA_INPUT}" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "schema hierarchy failed: ${error}")
endif()
string(JSON version GET "${output}" schema)
if(NOT version STREQUAL "snt-schema-hierarchy/1")
  message(FATAL_ERROR "Unexpected schema hierarchy version: ${version}")
endif()
string(JSON available GET "${output}" definitions_available)
string(JSON complete GET "${output}" applications_complete)
string(JSON definition_count LENGTH "${output}" definitions)
string(JSON application_count LENGTH "${output}" applications)
string(JSON nested_ref GET "${output}" definitions 1 members 1 schema_refs 0)
string(JSON source_file GET "${output}" definitions 0 origin file)
string(JSON item_path GET "${output}" applications 1 path)
string(JSON inherited GET "${output}" applications 1 inherited_from_collection)
string(JSON explicit_schema GET "${output}" applications 2 schema_ids 0)
string(JSON contributing GET "${output}" values 0 contributing_schema_id)
if(NOT available OR NOT complete OR NOT definition_count EQUAL 4 OR
   NOT application_count EQUAL 6 OR NOT nested_ref STREQUAL "address" OR
   NOT source_file STREQUAL "schema_hierarchy.dip" OR
   NOT item_path STREQUAL "people[alice]" OR NOT inherited OR
   NOT explicit_schema STREQUAL "role" OR NOT contributing STREQUAL "person")
  message(FATAL_ERROR "Schema hierarchy content is wrong: ${output}")
endif()

set(SCHEMA_SNAPSHOT "${CMAKE_CURRENT_BINARY_DIR}/schema-hierarchy-test.diph5")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip parse -i file "${SCHEMA_INPUT}" --save "${SCHEMA_SNAPSHOT}"
  RESULT_VARIABLE status OUTPUT_VARIABLE saved ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "schema snapshot save failed: ${error}")
endif()
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip schemas --input "${SCHEMA_SNAPSHOT}" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${SCHEMA_SNAPSHOT}")
if(NOT status EQUAL 0)
  message(FATAL_ERROR "schema snapshot inspection failed: ${error}")
endif()
string(JSON available GET "${output}" definitions_available)
string(JSON complete GET "${output}" applications_complete)
string(JSON definition_count LENGTH "${output}" definitions)
string(JSON inherited_type TYPE "${output}" applications 1 inherited_from_collection)
string(JSON snapshot_schema GET "${output}" applications 1 schema_ids 0)
if(available OR complete OR NOT definition_count EQUAL 0 OR
   NOT inherited_type STREQUAL "NULL" OR NOT snapshot_schema STREQUAL "person")
  message(FATAL_ERROR "Schema snapshot availability is wrong: ${output}")
endif()
