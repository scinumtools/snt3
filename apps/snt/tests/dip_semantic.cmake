cmake_policy(SET CMP0054 NEW)

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

set(SOURCE_INPUT "${CMAKE_CURRENT_BINARY_DIR}/source-metadata-test.dip")
file(WRITE "${SOURCE_INPUT}" "?authors \"Sample authors\"\n?title \"\"\nconcept str = \"sample\"\n")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip sources --input "${SOURCE_INPUT}" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${SOURCE_INPUT}")
if(NOT status EQUAL 0)
  message(FATAL_ERROR "source inspection failed: ${error}")
endif()
string(JSON source_count LENGTH "${output}" sources)
set(source_metadata_found FALSE)
if(source_count GREATER 0)
  math(EXPR source_last "${source_count} - 1")
  foreach(index RANGE 0 ${source_last})
    string(JSON authors GET "${output}" sources ${index} metadata authors)
    if(authors STREQUAL "Sample authors")
      string(JSON title GET "${output}" sources ${index} metadata title)
      string(JSON present_count LENGTH "${output}" sources ${index} metadata present)
      set(has_title FALSE)
      if(present_count GREATER 0)
        math(EXPR present_last "${present_count} - 1")
        foreach(present_index RANGE 0 ${present_last})
          string(JSON present_name GET "${output}" sources ${index} metadata present ${present_index})
          if(present_name STREQUAL "title")
            set(has_title TRUE)
          endif()
        endforeach()
      endif()
      if(title STREQUAL "" AND has_title)
        set(source_metadata_found TRUE)
      endif()
    endif()
  endforeach()
endif()
if(NOT source_metadata_found)
  message(FATAL_ERROR "Source metadata or explicit-empty presence is wrong: ${output}")
endif()

set(SCHEMA_METADATA_INPUT "${CMAKE_CURRENT_BINARY_DIR}/schema-instance-metadata-test.dip")
file(WRITE "${SCHEMA_METADATA_INPUT}"
  "$schema evidence\n"
  "  source_id str\n"
  "    ?category \"Citation\"\n"
  "    ?rationale \"Schema rationale\"\n"
  "records map : evidence\n"
  "records[first]\n"
  "  source_id = \"source_a\"\n"
  "    ?rationale \"Local rationale\"\n")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip describe --input "${SCHEMA_METADATA_INPUT}"
    --path "records[first].source_id" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${SCHEMA_METADATA_INPUT}")
if(NOT status EQUAL 0)
  message(FATAL_ERROR "schema field metadata inspection failed: ${error}")
endif()
string(JSON local_rationale GET "${output}" metadata rationale)
string(JSON inherited_category GET "${output}" metadata category)
if(NOT local_rationale STREQUAL "Local rationale" OR NOT inherited_category STREQUAL "Citation")
  message(FATAL_ERROR "Schema and instance metadata were not combined: ${output}")
endif()

set(METADATA_INPUT "${CMAKE_CURRENT_BINARY_DIR}/json-metadata-test.dip")
set(METADATA_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/json-metadata-test.json")
file(WRITE "${METADATA_INPUT}"
  "?authors \"Sample team\"\n"
  "$schema edge\n"
  "  source_id str\n"
  "    ?rationale \"Schema explanation\"\n"
  "edges map\n"
  "  ?descr \"Edge collection\"\n"
  "edges[uses_material] : edge\n"
  "  ?descr \"Material relationship\"\n"
  "  source_id = \"material_1\"\n"
  "    ?rationale \"Local explanation\"\n"
  "measurements table = \"\"\"\n"
  "?descr \"Measurement table\"\n"
  "time float s\n"
  "  ?descr \"Elapsed time\"\n"
  "---\n"
  "0\n"
  "\"\"\"\n"
  "empty str = \"\"\n"
  "  ?rationale \"\"\n"
  "absent str = none\n")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip parse -i file "${METADATA_INPUT}" --generate json-metadata "${METADATA_OUTPUT}"
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
file(REMOVE "${METADATA_INPUT}")
if(NOT status EQUAL 0)
  file(REMOVE "${METADATA_OUTPUT}")
  message(FATAL_ERROR "json-metadata generation failed: ${error}")
endif()
file(READ "${METADATA_OUTPUT}" output)
file(REMOVE "${METADATA_OUTPUT}")
string(JSON export_schema GET "${output}" schema)
string(JSON edge_value GET "${output}" values "edges[uses_material].source_id" value)
string(JSON edge_rationale GET "${output}" values "edges[uses_material].source_id" metadata values rationale)
string(JSON edge_collection_metadata GET "${output}" node_metadata edges metadata values description)
string(JSON edge_item_kind GET "${output}" node_metadata "edges[uses_material]" kind)
string(JSON edge_item_metadata GET "${output}" node_metadata "edges[uses_material]" metadata values description)
string(JSON table_kind GET "${output}" node_metadata measurements kind)
string(JSON table_metadata GET "${output}" node_metadata measurements metadata values description)
string(JSON column_kind GET "${output}" node_metadata measurements.time kind)
string(JSON column_metadata GET "${output}" node_metadata measurements.time metadata values description)
string(JSON empty_value GET "${output}" values empty value)
string(JSON empty_presence GET "${output}" values empty presence)
string(JSON empty_rationale GET "${output}" values empty metadata values rationale)
string(JSON empty_metadata_present GET "${output}" values empty metadata present 0)
string(JSON absent_presence GET "${output}" values absent presence)
string(JSON absent_value_type TYPE "${output}" values absent value)
string(JSON source_count LENGTH "${output}" sources)
set(source_authors_found FALSE)
if(source_count GREATER 0)
  math(EXPR source_last "${source_count} - 1")
  foreach(index RANGE 0 ${source_last})
    string(JSON source_name MEMBER "${output}" sources ${index})
    string(JSON source_metadata ERROR_VARIABLE source_error GET "${output}" sources "${source_name}" metadata values authors)
    if(NOT source_error AND source_metadata STREQUAL "Sample team")
      set(source_authors_found TRUE)
    endif()
  endforeach()
endif()
if(NOT export_schema STREQUAL "snt-dip-values/2" OR
   NOT edge_value STREQUAL "material_1" OR
   NOT edge_rationale STREQUAL "Local explanation" OR
   NOT edge_collection_metadata STREQUAL "Edge collection" OR
   NOT edge_item_kind STREQUAL "map_item" OR
   NOT edge_item_metadata STREQUAL "Material relationship" OR
   NOT table_kind STREQUAL "table" OR
   NOT table_metadata STREQUAL "Measurement table" OR
   NOT column_kind STREQUAL "table_column" OR
   NOT column_metadata STREQUAL "Elapsed time" OR
   NOT empty_value STREQUAL "" OR
   NOT empty_presence STREQUAL "value" OR
   NOT empty_rationale STREQUAL "" OR
   NOT empty_metadata_present STREQUAL "rationale" OR
   NOT absent_presence STREQUAL "none" OR NOT absent_value_type STREQUAL "NULL" OR
   NOT source_authors_found)
  message(FATAL_ERROR "Structured values, metadata, presence, or source provenance is wrong: ${output}")
endif()

set(SCHEMA_INPUT "${SOURCE_DIR}/tests/dip/fixtures/schema_hierarchy.dip")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip schemas --input "${SCHEMA_INPUT}" --format json
  RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "schema hierarchy failed: ${error}")
endif()
string(JSON version GET "${output}" schema)
if(NOT version STREQUAL "snt-schema-hierarchy/2")
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
