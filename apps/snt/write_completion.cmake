execute_process(
  COMMAND "${SNT_EXECUTABLE}" completion "${SHELL}"
  OUTPUT_FILE "${OUTPUT_FILE}"
  RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "Failed to generate ${SHELL} completion for snt")
endif()
