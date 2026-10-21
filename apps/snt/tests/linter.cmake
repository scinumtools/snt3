file(MAKE_DIRECTORY "${TEST_DIR}")
set(clean "${TEST_DIR}/clean.dip")
set(advisory "${TEST_DIR}/advisory.dip")
set(invalid "${TEST_DIR}/invalid.dip")
set(manifest "${TEST_DIR}/DIPfile")
set(choices "${TEST_DIR}/choices.dip")
set(optin_dir "${TEST_DIR}/optin")
file(WRITE "${clean}" "steps int = 8\n")
file(WRITE "${advisory}" "steps int = 8\n  ?visibility \"public\"\nold int = 3\n  ?deprecated \"Use steps\"\n")
file(WRITE "${invalid}" "bad unknown_type = 1\nother unknown_type = 2\n")
file(WRITE "${choices}" "level int = 1\n  ?visibility \"public\"\n  !options [1, 1, 2]\n")
file(WRITE "${manifest}" "lint\n  require_public_descriptions = false\n  warn_deprecated_values = true\ncode[]\n  file = \"advisory.dip\"\n")
file(MAKE_DIRECTORY "${optin_dir}")
file(WRITE "${optin_dir}/DIPfile"
  "lint\n  warn_broken_metadata_references = true\n  warn_unused_schemas = true\ncode[]\n  file = \"optional.dip\"\n")
file(WRITE "${optin_dir}/optional.dip"
  "$schema spare\n  value int = 1\nroot\n  child int = 1\ncurrent int = 2\n  ?requires \"root.missing\"\n")

execute_process(COMMAND "${SNT_EXECUTABLE}" lint "${clean}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 0 OR NOT out MATCHES "clean")
  message(FATAL_ERROR "Clean file failed lint: ${status} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" lint --json "${advisory}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON input_valid GET "${out}" valid)
string(JSON clean_result GET "${out}" clean)
string(JSON finding_count LENGTH "${out}" diagnostics)
string(JSON code GET "${out}" diagnostics 0 code)
string(JSON line GET "${out}" diagnostics 0 location line)
if(NOT status EQUAL 1 OR NOT input_valid OR clean_result OR NOT finding_count EQUAL 1 OR
   NOT code STREQUAL "dip.lint.deprecated-value" OR NOT line EQUAL 3)
  message(FATAL_ERROR "DIPfile lint setting or JSON diagnostic failed: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" lint --json "${manifest}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON code GET "${out}" diagnostics 0 code)
if(NOT status EQUAL 1 OR NOT code STREQUAL "dip.lint.deprecated-value")
  message(FATAL_ERROR "Project lint failed: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" lint --json "${invalid}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON input_valid GET "${out}" valid)
string(JSON finding_count LENGTH "${out}" diagnostics)
if(NOT status EQUAL 1 OR input_valid OR NOT finding_count EQUAL 2)
  message(FATAL_ERROR "Invalid DIPL did not reuse multi-error validation: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" lint --json "${choices}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON finding_count LENGTH "${out}" diagnostics)
string(JSON first_code GET "${out}" diagnostics 0 code)
string(JSON second_code GET "${out}" diagnostics 1 code)
if(NOT status EQUAL 1 OR NOT finding_count EQUAL 2 OR
   NOT first_code STREQUAL "dip.lint.duplicate-options" OR
   NOT second_code STREQUAL "dip.lint.public-option-labels")
  message(FATAL_ERROR "Default choice rules failed: ${out} ${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" lint --json "${optin_dir}/DIPfile"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
string(JSON finding_count LENGTH "${out}" diagnostics)
string(JSON first_code GET "${out}" diagnostics 0 code)
string(JSON second_code GET "${out}" diagnostics 1 code)
if(NOT status EQUAL 1 OR NOT finding_count EQUAL 2 OR
   NOT first_code STREQUAL "dip.lint.broken-metadata-reference" OR
   NOT second_code STREQUAL "dip.lint.unused-schema")
  message(FATAL_ERROR "Optional DIPfile lint rules failed: ${out} ${err}")
endif()
