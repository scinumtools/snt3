file(MAKE_DIRECTORY "${TEST_DIR}")
set(valid "${TEST_DIR}/valid.dip")
set(invalid "${TEST_DIR}/invalid.dip")
set(multiple "${TEST_DIR}/multiple.dip")
set(manifest "${TEST_DIR}/DIPfile")
set(invalid_manifest "${TEST_DIR}/invalid.manifest")
set(settings_dir "${TEST_DIR}/settings")
set(invalid_settings_dir "${TEST_DIR}/invalid-settings")
set(renamed_manifest "${TEST_DIR}/experiment.manifest")
file(WRITE "${valid}" "answer int = 42\n")
file(WRITE "${invalid}" "answer unknown_type = 42\n")
file(WRITE "${multiple}" "first unknown_type = 1\nvalid int = 2\nsecond unknown_type = 3\n")
file(WRITE "${manifest}" "code[]\n  file = \"valid.dip\"\n")
file(WRITE "${renamed_manifest}" "code[]\n  file = \"valid.dip\"\n")
file(WRITE "${invalid_manifest}" "first unknown_type = 1\nsecond unknown_type = 2\n")
file(MAKE_DIRECTORY "${settings_dir}" "${invalid_settings_dir}")
file(WRITE "${settings_dir}/DIPfile"
  "validate\n  max_syntax_errors = 1\ncode[]\n  file = \"limited.dip\"\n")
file(WRITE "${settings_dir}/limited.dip"
  "first unknown_type = 1\nsecond unknown_type = 2\n")
file(WRITE "${invalid_settings_dir}/DIPfile"
  "validate\n  max_syntax_errors = 0\n")

execute_process(COMMAND "${SNT_EXECUTABLE}" validate "${valid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0 OR NOT out MATCHES "valid")
  message(FATAL_ERROR "Valid file failed: status=${status}, ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate "${invalid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1 OR NOT err MATCHES "error \\[dip\\.")
  message(FATAL_ERROR "Invalid DIPL did not produce a diagnostic: status=${status}, ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate "${manifest}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Valid DIPfile failed: status=${status}, ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --project "${renamed_manifest}"
  RESULT_VARIABLE status ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Explicit project manifest failed: status=${status}, ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${invalid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1)
  message(FATAL_ERROR "JSON validation returned ${status}: ${err}")
endif()
string(JSON json_valid GET "${out}" valid)
string(JSON json_code GET "${out}" diagnostics 0 code)
if(json_valid OR NOT json_code MATCHES "^dip\\.")
  message(FATAL_ERROR "JSON diagnostic contract failed: ${out}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${multiple}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1)
  message(FATAL_ERROR "Multi-error validation returned ${status}: ${err}")
endif()
string(JSON diagnostic_count LENGTH "${out}" diagnostics)
string(JSON first_line GET "${out}" diagnostics 0 location line)
string(JSON second_line GET "${out}" diagnostics 1 location line)
if(NOT diagnostic_count EQUAL 2 OR NOT first_line EQUAL 1 OR NOT second_line EQUAL 3)
  message(FATAL_ERROR "Independent syntax errors were not collected: ${out}")
endif()

file(WRITE "${manifest}" "code[]\n  file = \"multiple.dip\"\n")
execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${manifest}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON project_count LENGTH "${out}" diagnostics)
string(JSON project_source GET "${out}" diagnostics 0 location source)
file(TO_CMAKE_PATH "${project_source}" project_source_normalized)
file(TO_CMAKE_PATH "${multiple}" multiple_normalized)
if(NOT status EQUAL 1 OR NOT project_count EQUAL 2 OR
   NOT "${project_source_normalized}" STREQUAL "${multiple_normalized}")
  message(FATAL_ERROR "Project code syntax errors were not collected: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --project --json "${invalid_manifest}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON manifest_count LENGTH "${out}" diagnostics)
if(NOT status EQUAL 1 OR NOT manifest_count EQUAL 2)
  message(FATAL_ERROR "Manifest syntax errors were not collected: ${out} ${err}")
endif()

foreach(limited_input IN ITEMS "${settings_dir}/limited.dip" "${settings_dir}/DIPfile")
  execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${limited_input}"
    RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
  string(JSON limited_count LENGTH "${out}" diagnostics)
  if(NOT status EQUAL 1 OR NOT limited_count EQUAL 1)
    message(FATAL_ERROR "DIPfile validation limit was ignored for ${limited_input}: ${out} ${err}")
  endif()
endforeach()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${invalid_settings_dir}/DIPfile"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON invalid_setting_line GET "${out}" diagnostics 0 location line)
string(JSON invalid_setting_source GET "${out}" diagnostics 0 location source)
file(TO_CMAKE_PATH "${invalid_setting_source}" invalid_setting_source_normalized)
file(TO_CMAKE_PATH "${invalid_settings_dir}/DIPfile" invalid_setting_expected_normalized)
if(NOT status EQUAL 1 OR NOT invalid_setting_line EQUAL 2 OR
   NOT "${invalid_setting_source_normalized}" STREQUAL "${invalid_setting_expected_normalized}" OR
   NOT out MATCHES "Invalid validation setting")
  message(FATAL_ERROR "Invalid DIPfile validation limit was not rejected: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate "${valid}" "${invalid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1 OR NOT out MATCHES "valid" OR NOT err MATCHES "invalid")
  message(FATAL_ERROR "Validation did not continue across inputs: status=${status}, ${out}, ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate --json "${valid}" "${invalid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1)
  message(FATAL_ERROR "Multi-input JSON validation returned ${status}: ${err}")
endif()
string(JSON first_valid GET "${out}" 0 valid)
string(JSON second_valid GET "${out}" 1 valid)
if(NOT first_valid OR second_valid)
  message(FATAL_ERROR "Multi-input JSON result order is wrong: ${out}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" validate "${TEST_DIR}/missing.dip"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 2 OR NOT err MATCHES "dip.io")
  message(FATAL_ERROR "Missing file was not reported as input error: status=${status}, ${err}")
endif()
