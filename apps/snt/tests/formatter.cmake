file(MAKE_DIRECTORY "${TEST_DIR}")
set(manifest "${TEST_DIR}/DIPfile")
set(input "${TEST_DIR}/parameters.dip")
file(WRITE "${manifest}" "format\n  blank_lines_between_nodes = 0\n  final_newline = true\n")
file(WRITE "${input}" "a int=1\nb int=2\n")

execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --input file "${input}" --print
  RESULT_VARIABLE parse_status OUTPUT_VARIABLE before_values ERROR_VARIABLE parse_error)
if(NOT parse_status EQUAL 0)
  message(FATAL_ERROR "Original DIPL did not parse: ${parse_error}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" format --check "${input}"
  RESULT_VARIABLE status OUTPUT_VARIABLE out ERROR_VARIABLE err)
if(NOT status EQUAL 1)
  message(FATAL_ERROR "Expected --check to find changes; status=${status}, stderr=${err}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" format --write "${input}"
  RESULT_VARIABLE status ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "--write failed: ${err}")
endif()
file(READ "${input}" actual)
if(NOT actual STREQUAL "a int = 1\nb int = 2\n")
  message(FATAL_ERROR "Unexpected formatted text: ${actual}")
endif()
execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --input file "${input}" --print
  RESULT_VARIABLE parse_status OUTPUT_VARIABLE after_values ERROR_VARIABLE parse_error)
if(NOT parse_status EQUAL 0 OR NOT before_values STREQUAL after_values)
  message(FATAL_ERROR "Formatting changed evaluated values: ${parse_error}")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" format --check "${input}"
  RESULT_VARIABLE status ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Expected formatted file to pass --check: ${err}")
endif()

# A standalone file outside any DIPfile tree uses the built-in defaults.
set(standalone "${TEST_DIR}/../standalone-formatter.dip")
file(WRITE "${standalone}" "first int=1\nsecond int=2\n")
# Keep the former shorthand working for existing scripts without advertising it.
execute_process(COMMAND "${SNT_EXECUTABLE}" fmt "${standalone}"
  RESULT_VARIABLE status OUTPUT_VARIABLE actual ERROR_VARIABLE err)
if(NOT status EQUAL 0 OR NOT actual STREQUAL "first int = 1\n\nsecond int = 2\n")
  string(HEX "${actual}" actual_hex)
  message(FATAL_ERROR "Standalone formatter defaults failed: ${err}; output_hex=${actual_hex}")
endif()

# Stdout must preserve CRLF input exactly, including on Windows where the C
# runtime otherwise translates every LF written through a text-mode stream.
set(crlf "${TEST_DIR}/../standalone-formatter-crlf.dip")
set(crlf_output "${TEST_DIR}/../standalone-formatter-crlf-output.dip")
set(crlf_expected "${TEST_DIR}/../standalone-formatter-crlf-expected.dip")
file(WRITE "${crlf}" "first int=1\r\nsecond int=2\r\n")
file(WRITE "${crlf_expected}" "first int = 1\r\n\r\nsecond int = 2\r\n")
execute_process(COMMAND "${SNT_EXECUTABLE}" format "${crlf}"
  RESULT_VARIABLE status OUTPUT_FILE "${crlf_output}" ERROR_VARIABLE err)
file(SHA256 "${crlf_output}" actual_hash)
file(SHA256 "${crlf_expected}" expected_hash)
if(NOT status EQUAL 0 OR NOT actual_hash STREQUAL expected_hash)
  message(FATAL_ERROR "CRLF formatter stdout changed line endings: ${err}")
endif()
