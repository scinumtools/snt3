file(MAKE_DIRECTORY "${TEST_DIR}")
set(project_dir "${TEST_DIR}/project")
file(MAKE_DIRECTORY "${project_dir}")
file(WRITE "${project_dir}/settings.dipl"
  "?descr \"Reusable settings\"\n?title \"Schema & paper\"\nspeed float = 2 m/s\n  ?descr \"Flow & speed_#%\"\n")
file(WRITE "${project_dir}/parameters.dip"
  "physics : settings\nname str = \"A&B_#%\"\n  ?title \"Value study\"\n  ?doi \"10.1/example_#\"\n")
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
    "draft\\_1" "Introduction" "measured" "physics.speed" "3" "m/s" "Overridden" "Override at"
    "Reusable settings" "Schema \\& paper" "Flow \\& speed\\_\\#\\%" "Value study"
    "10.1/example\\_\\#" "A\\&B\\_\\#\\%" "Sources"
    "Custom units" "\\sntnode{settings}" "\\sntnode{custom\\_length}"
    "\\sntnode{DIP0}" "2*m" "parameters.dip")
  string(FIND "${tex}" "${expected}" index)
  if(index EQUAL -1)
    message(FATAL_ERROR "TeX report is missing '${expected}'")
  endif()
endforeach()
if(tex MATCHES "\\\\section\\*\\{Hierarchy\\}")
  message(FATAL_ERROR "Report still contains a hierarchy section")
endif()
string(FIND "${tex}" "name" name_pos)
string(FIND "${tex}" "physics.speed" speed_pos)
if(name_pos GREATER speed_pos)
  message(FATAL_ERROR "Parameter order is not stable and sorted")
endif()

execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --project "${project_dir}/DIPfile"
  -i override_string "physics.speed = 3 m/s" --save "${snapshot}"
  RESULT_VARIABLE save_status ERROR_VARIABLE save_error)
if(NOT save_status EQUAL 0)
  message(FATAL_ERROR "Could not save test environment: ${save_error}")
endif()
run_report(success --load "${snapshot}" --output "${TEST_DIR}/loaded.tex")
file(READ "${TEST_DIR}/loaded.tex" loaded)
foreach(expected IN ITEMS "physics.speed" "Overridden" "Override at" "Reusable settings"
    "Schema \\& paper" "Sources" "Custom units" "custom\\_length")
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
