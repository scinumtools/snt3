file(MAKE_DIRECTORY "${TEST_DIR}")
set(saved "${TEST_DIR}/parameters with spaces.diph5")
set(copied "${TEST_DIR}/copy.diph5")

function(run_cli expected_status expected_output)
  execute_process(
    COMMAND "${SNT_EXECUTABLE}" dip parse ${ARGN}
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error
  )
  if(expected_status STREQUAL "success")
    if(NOT status EQUAL 0 OR NOT output STREQUAL expected_output)
      message(FATAL_ERROR "Command failed (${status}): ${ARGN}\nstdout: ${output}\nstderr: ${error}")
    endif()
  elseif(status EQUAL 0 OR error STREQUAL "")
    message(FATAL_ERROR "Expected a failure with stderr: ${ARGN}\nstdout: ${output}")
  endif()
endfunction()

run_cli(success "" --input string "steps int = 100\nenabled bool = true\n  !tags [\"selected\"]\n"
  --save "${saved}")
if(NOT EXISTS "${saved}")
  message(FATAL_ERROR "Save did not create a DIPH5 file")
endif()
run_cli(success "100\n" --load "${saved}" --request steps --value --type integer)
run_cli(success "enabled = true\n" --load "${saved}" --tags selected --print --save "${copied}")
run_cli(success "100\n" --load "${copied}" --request steps --value --type integer)
run_cli(success "" --load "${copied}" --save "${copied}")
run_cli(success "100\n" --load "${copied}" --request steps --value)

# Failed output validation must leave an existing destination untouched.
run_cli(failure "" --input string "steps int = 2" --request steps --value --type bool --save "${saved}")
run_cli(success "100\n" --load "${saved}" --request steps --value)

run_cli(failure "" --load "${saved}" --input string "other int = 1")
run_cli(failure "" --input string "other int = 1" --load "${saved}")
run_cli(failure "" --load)
run_cli(failure "" --load "${saved}" "${copied}")
run_cli(failure "" --load "${saved}" --load "${saved}")
run_cli(failure "" --load "${saved}" --save)
run_cli(failure "" --load "${saved}" --save "${copied}" "${saved}")
run_cli(failure "" --load "${saved}" --save "${copied}" --save "${copied}")
run_cli(failure "" --save "${saved}")
run_cli(failure "" --load "${TEST_DIR}/missing.diph5")
run_cli(failure "" --load "${saved}" --save "${saved}/invalid.diph5")

# Saving overwrites an existing file.
run_cli(success "" --input string "steps int = 2" --save "${saved}")
run_cli(success "2\n" --load "${saved}" --request steps --value)
file(REMOVE "${saved}" "${copied}")
