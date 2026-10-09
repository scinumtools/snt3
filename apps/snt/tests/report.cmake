file(MAKE_DIRECTORY "${TEST_DIR}")
set(project_dir "${TEST_DIR}/project")
file(MAKE_DIRECTORY "${project_dir}")
file(WRITE "${project_dir}/settings.dipl"
  "?descr \"Reusable settings\"\n?title \"Schema & paper\"\nspeed float = 2 m/s\n  ?descr \"Flow & speed_#%\"\n  !options [2, 3] m/s\n  !condition ({.} > 0 m/s)\n  ?rationale \"Measured at startup\"\n  ?recommended_range \"2 to 3 m/s\"\n  ?scientific_impact \"Controls transport speed\"\n")
file(WRITE "${project_dir}/parameters.dip"
  "physics : settings\ndouble_speed float = ({?physics.speed} * 2) m/s\nname str = \"A&B_#%\"\n  ?title \"Value study\"\n  ?doi \"10.1/example_#\"\nduration float = 12 h\nduration = 14 h\nmeasurements table = \"\"\"temperature float K\ntime float s\n---\n295 0\n296 1\n\"\"\"\n")
file(WRITE "${project_dir}/DIPfile"
  "units[]\n  name = \"custom_length\"\n  unit = \"2*m\"\nschemas[]\n  name = \"settings\"\n  file = \"settings.dipl\"\ncode[]\n  file = \"parameters.dip\"\n")
file(WRITE "${project_dir}/intro.tex" "This model uses \\textbf{measured} values.\n")
set(report "${TEST_DIR}/report.tex")
set(snapshot "${TEST_DIR}/snapshot.diph5")

function(run_report expected_status)
  execute_process(COMMAND "${SNT_EXECUTABLE}" report ${ARGN}
    RESULT_VARIABLE status OUTPUT_VARIABLE stdout ERROR_VARIABLE stderr)
  if(expected_status STREQUAL "success")
    if(NOT status EQUAL 0)
      message(FATAL_ERROR "snt report failed (${status}): ${ARGN}\n${stderr}")
    endif()
  elseif(status EQUAL 0 OR stderr STREQUAL "")
    message(FATAL_ERROR "Expected snt report to fail: ${ARGN}\n${stdout}")
  endif()
  set(LAST_ERROR "${stderr}" PARENT_SCOPE)
endfunction()

run_report(success --project "${project_dir}/DIPfile"
  -i override_string "physics.speed = 3 m/s"
  --intro "${project_dir}/intro.tex" --output "${report}"
  --title "Demo & report" --author "Ada_Lovelace" --date "2026-09-28"
  --report-version "draft_1")
file(READ "${report}" tex)
string(FIND "${tex}" "${project_dir}/parameters.dip" absolute_source_pos)
if(NOT absolute_source_pos EQUAL -1)
  message(FATAL_ERROR "Project report leaked an absolute source path")
endif()
foreach(expected IN ITEMS "\\begin{titlepage}" "\\tableofcontents" "\\usepackage{hyperref}"
    "\\section{Parameters}" "Demo \\& report" "Ada\\_Lovelace" "2026-09-28"
    "draft\\_1" "Introduction" "measured" "physics.speed" "3" "m/s" "Override at"
    "Modified at" "Modification" "\\section{Tables}" "\\sntnode{measurements}" "Column 1"
    "Reusable settings" "Schema \\& paper" "Flow \\& speed\\_\\#\\%" "Value study"
    "10.1/example\\_\\#" "A\\&B\\_\\#\\%" "Sources"
    "Custom units" "\\sntnode{settings}" "\\sntnode{custom\\_length}"
    "Parameter guide" "\\hyperlink{snt-parameter-" "double\\_speed" "Reads during evaluation"
    "?physics.speed" "Used by" "Supplied parameters" "Supplied by schema"
    "Applied overrides" "Allowed options" "Validation condition"
    "Author guidance" "Measured at startup" "Recommended range"
    "Controls transport speed"
    "\\sntnode{DIP0}" "2*m" "parameters.dip")
  string(FIND "${tex}" "${expected}" index)
  if(index EQUAL -1)
    message(FATAL_ERROR "TeX report is missing '${expected}'")
  endif()
endforeach()
string(REGEX MATCH "Content hash & SHA-256: ([0-9a-f]+)[.][.][.]" abbreviated_hash "${tex}")
if(NOT abbreviated_hash)
  message(FATAL_ERROR "TeX report is missing an abbreviated source hash")
endif()
string(LENGTH "${CMAKE_MATCH_1}" hash_length)
if(NOT hash_length EQUAL 16)
  message(FATAL_ERROR "Source hash prefix should contain 16 characters, got ${hash_length}")
endif()
if(tex MATCHES "Overridden")
  message(FATAL_ERROR "Override location already conveys status; report should not repeat it")
endif()
if(tex MATCHES "\\\\section\\*\\{Hierarchy\\}")
  message(FATAL_ERROR "Report still contains a hierarchy section")
endif()
string(FIND "${tex}" "name" name_pos)
string(FIND "${tex}" "physics.speed" speed_pos)
if(name_pos GREATER speed_pos)
  message(FATAL_ERROR "Parameter order is not stable and sorted")
endif()

foreach(format IN ITEMS md rst html typ txt json)
  set(rendered "${TEST_DIR}/report.${format}")
  run_report(success --project "${project_dir}/DIPfile"
    -i override_string "physics.speed = 3 m/s"
    --format "${format}" --output "${rendered}"
    --date "2026-09-28" --report-version "draft_1")
  file(READ "${rendered}" result)
  foreach(expected IN ITEMS "physics.speed" "Override at" "Modified at" "Tables" "Column 1" "Custom units" "Flow"
      "Parameter guide" "double" "Reads during evaluation" "Supplied parameters"
      "Applied overrides" "Allowed options" "Validation condition"
      "Measured at startup" "Recommended range" "Controls transport speed")
    string(FIND "${result}" "${expected}" index)
    if(index EQUAL -1)
      message(FATAL_ERROR "${format} report is missing '${expected}'")
    endif()
  endforeach()
  if(format STREQUAL "json")
    string(REGEX MATCH "SHA-256: ([0-9a-f]+)" json_hash "${result}")
    string(LENGTH "${CMAKE_MATCH_1}" json_hash_length)
    if(NOT json_hash_length EQUAL 64)
      message(FATAL_ERROR "JSON report should retain the full SHA-256 digest")
    endif()
  endif()
  if(format STREQUAL "html")
    if(NOT result MATCHES "href=\"#snt-parameter-[0-9]+\"" OR
       NOT result MATCHES "id=\"snt-parameter-[0-9]+\"")
      message(FATAL_ERROR "HTML parameter guide links do not target parameter entries")
    endif()
  endif()
endforeach()
run_report(failure --project "${project_dir}/DIPfile" --intro "${project_dir}/intro.tex"
  --format html --output "${TEST_DIR}/bad.html")
if(NOT LAST_ERROR MATCHES "--intro requires")
  message(FATAL_ERROR "HTML report did not reject the LaTeX introduction")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --project "${project_dir}/DIPfile"
  -i override_string "physics.speed = 3 m/s" --save "${snapshot}"
  RESULT_VARIABLE save_status ERROR_VARIABLE save_error)
if(NOT save_status EQUAL 0)
  message(FATAL_ERROR "Could not save test environment: ${save_error}")
endif()
run_report(success --load "${snapshot}" --output "${TEST_DIR}/loaded.tex")
file(READ "${TEST_DIR}/loaded.tex" loaded)
foreach(expected IN ITEMS "physics.speed" "Override at" "Modified at" "Tables" "Column 1" "Reusable settings"
    "Schema \\& paper" "Sources" "Custom units" "custom\\_length"
    "Applied overrides" "Allowed options" "Validation condition" "Recommended range")
  string(FIND "${loaded}" "${expected}" index)
  if(index EQUAL -1)
    message(FATAL_ERROR "DIPH5 report is missing '${expected}'")
  endif()
endforeach()

# The direct input path uses the same parser and produces evaluated values.
run_report(success --input string "count int = 7" --output "${TEST_DIR}/inline.tex")
file(READ "${TEST_DIR}/inline.tex" inline)
if(NOT inline MATCHES "count" OR NOT inline MATCHES "Value.*7")
  message(FATAL_ERROR "Inline DIPL input is missing from its report")
endif()
if(inline MATCHES "Overridden|Override at")
  message(FATAL_ERROR "An unmodified parameter should not show override status")
endif()

if(UNIX)
  # Capture the exact TeX sent to the PDF adapter and verify parity with --format tex.
  set(fake "${TEST_DIR}/fake-pdflatex")
  set(captured "${TEST_DIR}/compiled-source.tex")
  file(WRITE "${TEST_DIR}/compiler-passes.txt" "")
  file(WRITE "${fake}" "#!/bin/sh\nfor argument do\n  case \"$argument\" in\n    -output-directory=*) directory=\"\${argument#-output-directory=}\" ;;\n    *.tex) source=\"$argument\" ;;\n  esac\ndone\ncp \"$source\" \"${captured}\"\nprintf 'pass\\n' >> \"${TEST_DIR}/compiler-passes.txt\"\nprintf '%%PDF-fake\\n' > \"$directory/report.pdf\"\n")
  file(CHMOD "${fake}" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE)
  run_report(success --project "${project_dir}/DIPfile"
    -i override_string "physics.speed = 3 m/s"
    --intro "${project_dir}/intro.tex" --format pdf --output "${TEST_DIR}/fake.pdf"
    --title "Demo & report" --author "Ada_Lovelace" --date "2026-09-28"
    --report-version "draft_1"
    --tex-compiler "${fake}")
  file(READ "${captured}" compiled_source)
  if(NOT compiled_source STREQUAL tex)
    message(FATAL_ERROR "PDF adapter did not compile the same TeX as --format tex")
  endif()
  file(STRINGS "${TEST_DIR}/compiler-passes.txt" passes)
  list(LENGTH passes pass_count)
  if(NOT pass_count EQUAL 2)
    message(FATAL_ERROR "PDF adapter must compile twice to resolve contents links")
  endif()
endif()

run_report(failure --load "${snapshot}" --format pdf --output "${TEST_DIR}/missing.pdf"
  --tex-compiler "${TEST_DIR}/no-such-tex-compiler")
if(NOT LAST_ERROR MATCHES "unavailable")
  message(FATAL_ERROR "Missing compiler did not produce a clear error: ${LAST_ERROR}")
endif()
if(EXISTS "${TEST_DIR}/missing.pdf")
  message(FATAL_ERROR "Missing compiler left a PDF output")
endif()

find_program(PDFLATEX_EXECUTABLE pdflatex)
if(PDFLATEX_EXECUTABLE)
  run_report(success --project "${project_dir}/DIPfile" --format pdf
    --intro "${project_dir}/intro.tex" --output "${TEST_DIR}/report.pdf"
    --tex-compiler "${PDFLATEX_EXECUTABLE}")
  file(READ "${TEST_DIR}/report.pdf" signature LIMIT 4 HEX)
  if(NOT signature STREQUAL "25504446")
    message(FATAL_ERROR "PDF output has an invalid signature")
  endif()
endif()
