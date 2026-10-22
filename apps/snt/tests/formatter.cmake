file(MAKE_DIRECTORY "${TEST_DIR}")

# Render the bytes from file(READ ... HEX). A plain file(READ) normalizes CRLF
# on Windows and would hide the extra CR that this test needs to diagnose.
function(visible_hex hex result)
  set(visible "")
  string(LENGTH "${hex}" length)
  if(length GREATER 0)
    math(EXPR last_pair "${length} - 2")
    foreach(index RANGE 0 ${last_pair} 2)
      string(SUBSTRING "${hex}" ${index} 2 byte)
      if(byte STREQUAL "0d")
        string(APPEND visible "\\r")
      elseif(byte STREQUAL "0a")
        string(APPEND visible "\\n")
      elseif(byte STREQUAL "09")
        string(APPEND visible "\\t")
      elseif(byte STREQUAL "5c")
        string(APPEND visible "\\\\")
      else()
        math(EXPR code "0x${byte}")
        if(code GREATER_EQUAL 32 AND code LESS_EQUAL 126)
          string(ASCII ${code} character)
          string(APPEND visible "${character}")
        else()
          string(APPEND visible "\\x${byte}")
        endif()
      endif()
    endforeach()
  endif()
  set(${result} "${visible}" PARENT_SCOPE)
endfunction()

# Compare files as bytes: execute_process(OUTPUT_VARIABLE) can change line
# endings on Windows, so it cannot verify the formatter's stdout faithfully.
function(assert_same_bytes expected_path actual_path description)
  file(READ "${expected_path}" expected_hex HEX)
  file(READ "${actual_path}" actual_hex HEX)
  if(expected_hex STREQUAL actual_hex)
    return()
  endif()

  visible_hex("${expected_hex}" expected_text)
  visible_hex("${actual_hex}" actual_text)

  string(LENGTH "${expected_hex}" expected_length)
  string(LENGTH "${actual_hex}" actual_length)
  if(expected_length LESS actual_length)
    set(common_length "${expected_length}")
  else()
    set(common_length "${actual_length}")
  endif()
  math(EXPR first_difference "${common_length} / 2")
  if(common_length GREATER 0)
    math(EXPR last_pair "${common_length} - 2")
    foreach(index RANGE 0 ${last_pair} 2)
      string(SUBSTRING "${expected_hex}" ${index} 2 expected_byte)
      string(SUBSTRING "${actual_hex}" ${index} 2 actual_byte)
      if(NOT expected_byte STREQUAL actual_byte)
        math(EXPR first_difference "${index} / 2")
        break()
      endif()
    endforeach()
  endif()
  message(FATAL_ERROR
    "${description}; first differing byte: ${first_difference}\n"
    "  expected: \"${expected_text}\" (hex: ${expected_hex})\n"
    "  actual:   \"${actual_text}\" (hex: ${actual_hex})")
endfunction()

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
set(standalone_written "${TEST_DIR}/../standalone-formatter-written.dip")
set(standalone_output "${TEST_DIR}/../standalone-formatter-output.dip")
set(standalone_expected "${TEST_DIR}/../standalone-formatter-expected.dip")
file(WRITE "${standalone}" "first int=1\nsecond int=2\n")
file(WRITE "${standalone_expected}" "first int = 1\n\nsecond int = 2\n")
configure_file("${standalone}" "${standalone_written}" COPYONLY)
execute_process(COMMAND "${SNT_EXECUTABLE}" format --write "${standalone_written}"
  RESULT_VARIABLE status ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Standalone formatter --write failed: ${err}")
endif()
assert_same_bytes("${standalone_expected}" "${standalone_written}"
  "Standalone formatter defaults failed")
# Keep the former shorthand working for existing scripts without advertising it.
execute_process(COMMAND "${SNT_EXECUTABLE}" fmt "${standalone}"
  RESULT_VARIABLE status OUTPUT_FILE "${standalone_output}" ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Standalone formatter stdout failed: ${err}")
endif()
assert_same_bytes("${standalone_written}" "${standalone_output}"
  "Standalone formatter stdout differs from --write")

# For CRLF input, stdout must match the formatter's binary --write output.
# Windows text-mode stdout can otherwise add CR bytes to the result.
set(crlf "${TEST_DIR}/../standalone-formatter-crlf.dip")
set(crlf_written "${TEST_DIR}/../standalone-formatter-crlf-written.dip")
set(crlf_output "${TEST_DIR}/../standalone-formatter-crlf-output.dip")
set(crlf_expected "${TEST_DIR}/../standalone-formatter-crlf-expected.dip")
# CMake's file(WRITE) translates LF to CRLF on Windows. Supplying explicit
# CRLF there would create CRCRLF; Unix needs the explicit CR bytes instead.
if(WIN32)
  file(WRITE "${crlf}" "first int=1\nsecond int=2\n")
  file(WRITE "${crlf_expected}" "first int = 1\n\nsecond int = 2\n")
else()
  file(WRITE "${crlf}" "first int=1\r\nsecond int=2\r\n")
  file(WRITE "${crlf_expected}" "first int = 1\r\n\r\nsecond int = 2\r\n")
endif()
string(HEX "first int=1\r\nsecond int=2\r\n" crlf_fixture_hex)
file(READ "${crlf}" actual_fixture_hex HEX)
if(NOT actual_fixture_hex STREQUAL crlf_fixture_hex)
  visible_hex("${actual_fixture_hex}" actual_fixture)
  message(FATAL_ERROR "CRLF test fixture has wrong line endings: \"${actual_fixture}\"")
endif()
configure_file("${crlf}" "${crlf_written}" COPYONLY)
execute_process(COMMAND "${SNT_EXECUTABLE}" format --write "${crlf_written}"
  RESULT_VARIABLE status ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "CRLF formatter --write failed: ${err}")
endif()
assert_same_bytes("${crlf_expected}" "${crlf_written}"
  "CRLF formatter --write changed line endings or formatting")
execute_process(COMMAND "${SNT_EXECUTABLE}" format "${crlf}"
  RESULT_VARIABLE status OUTPUT_FILE "${crlf_output}" ERROR_VARIABLE err)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "CRLF formatter stdout failed: ${err}")
endif()
assert_same_bytes("${crlf_written}" "${crlf_output}"
  "CRLF formatter stdout differs from --write")
