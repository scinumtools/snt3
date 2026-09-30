file(MAKE_DIRECTORY "${TEST_DIR}")
set(before "${TEST_DIR}/before.diph5")
set(after "${TEST_DIR}/after.diph5")
execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --input string "value int = 1" --save "${before}"
  RESULT_VARIABLE save_before)
execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --input string "value int = 2" --save "${after}"
  RESULT_VARIABLE save_after)
if(NOT save_before EQUAL 0 OR NOT save_after EQUAL 0)
  message(FATAL_ERROR "Unable to create comparison fixtures")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" dip compare "${before}" "${before}"
  RESULT_VARIABLE equal_status OUTPUT_VARIABLE equal_output)
if(NOT equal_status EQUAL 0 OR NOT equal_output MATCHES "0 added, 0 removed, 0 changed")
  message(FATAL_ERROR "Equal comparison failed: ${equal_status} ${equal_output}")
endif()
execute_process(COMMAND "${SNT_EXECUTABLE}" dip compare "${before}" "${after}" --max-details 0
  RESULT_VARIABLE different_status OUTPUT_VARIABLE different_output)
if(NOT different_status EQUAL 1 OR NOT different_output MATCHES "1 changed" OR
   NOT different_output MATCHES "1 more differences")
  message(FATAL_ERROR "Different comparison failed: ${different_status} ${different_output}")
endif()
execute_process(COMMAND "${SNT_EXECUTABLE}" dip compare "${before}" "${TEST_DIR}/missing.diph5"
  RESULT_VARIABLE error_status ERROR_VARIABLE error_output)
if(NOT error_status EQUAL 2 OR error_output STREQUAL "")
  message(FATAL_ERROR "Invalid comparison did not report an error")
endif()
