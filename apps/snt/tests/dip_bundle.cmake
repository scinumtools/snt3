file(MAKE_DIRECTORY "${TEST_DIR}")
file(WRITE "${TEST_DIR}/DIPfile"
  "sources[]\n  name = \"data\"\n  filepath = \"data.dipt\"\n"
  "code[]\n  file = \"model.dip\"\n")
file(WRITE "${TEST_DIR}/data.dipt" "mass float kg\n---\n3\n")
file(WRITE "${TEST_DIR}/model.dip" "samples table = {data}\nscale float = 2\n")
execute_process(
  COMMAND "${SNT_EXECUTABLE}" dip package --project "${TEST_DIR}/DIPfile"
          --output "${TEST_DIR}/project.bundle.json"
  RESULT_VARIABLE result
  ERROR_VARIABLE error
)
if(NOT result EQUAL 0)
  message(FATAL_ERROR "snt dip package failed: ${error}")
endif()
file(READ "${TEST_DIR}/project.bundle.json" bundle)
string(JSON format GET "${bundle}" format)
string(JSON entry GET "${bundle}" entry)
string(JSON count LENGTH "${bundle}" files)
if(NOT format STREQUAL "snt-dipl-project-bundle/1" OR
   NOT entry STREQUAL "DIPfile" OR NOT count EQUAL 3)
  message(FATAL_ERROR "Unexpected project bundle: ${bundle}")
endif()
