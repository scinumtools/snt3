cmake_minimum_required(VERSION 3.22)
file(MAKE_DIRECTORY "${TEST_ROOT}")
set(input "${TEST_ROOT}/build config.dip")
file(WRITE "${input}" [=[build
  enabled bool = false
  count int = 17
  text str = "  a;b ${literal}  "
  values int[1] = [3]
  length float = 2 m
]=])
include("${SNT_HELPER}")
snt_dip_get(FILE "${input}" PATH build.enabled TYPE BOOL OUTPUT enabled)
if(enabled)
    message(FATAL_ERROR "DIPL false incorrectly enabled a CMake branch")
endif()
snt_dip_get(FILE "${input}" PATH "?build.count" TYPE INTEGER OUT count REQUIRED)
if(NOT count STREQUAL "17")
    message(FATAL_ERROR "Integer query failed: ${count}")
endif()
snt_dip_get(FILE "${input}" PATH build.text TYPE STRING OUTPUT text)
if(NOT text STREQUAL [=[  a;b ${literal}  ]=])
    message(FATAL_ERROR "String contents were changed: [${text}]")
endif()
foreach(request build.missing build.values build.length build)
    execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse -i file "${input}" -r "${request}" --value
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(status STREQUAL "0" OR NOT output STREQUAL "" OR error STREQUAL "")
        message(FATAL_ERROR "Invalid request ${request} did not fail cleanly: ${status}, ${output}, ${error}")
    endif()
endforeach()
execute_process(COMMAND "${SNT_EXECUTABLE}" dip parse --bogus
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(status STREQUAL "0" OR NOT output STREQUAL "" OR error STREQUAL "")
    message(FATAL_ERROR "Unknown CLI flag did not fail cleanly")
endif()

# Helper failures must not silently become a truthy diagnostic or an empty value.
foreach(arguments
    "PATH build.count TYPE BOOL OUTPUT result"
    "PATH build.missing OUTPUT result"
    "PATH build.count OUTPUT result TYPO typo"
    "PATH build.count TYPE BOOL"
)
    file(WRITE "${TEST_ROOT}/failure.cmake"
        "include(\"${SNT_HELPER}\")\nsnt_dip_get(FILE \"${input}\" ${arguments})\n")
    execute_process(COMMAND "${CMAKE_COMMAND}" "-DSNT_EXECUTABLE=${SNT_EXECUTABLE}"
        -P "${TEST_ROOT}/failure.cmake"
        RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
    if(status STREQUAL "0")
        message(FATAL_ERROR "Invalid helper query succeeded: ${arguments}")
    endif()
endforeach()

# Exercise actual configure/build regeneration, not just the helper in script mode.
set(source "${TEST_ROOT}/project")
set(binary "${TEST_ROOT}/build")
file(MAKE_DIRECTORY "${source}")
file(WRITE "${source}/config.dip" "feature bool = true\n")
file(WRITE "${source}/external.dip" "unused bool = true\n")
file(WRITE "${source}/CMakeLists.txt" [=[
cmake_minimum_required(VERSION 3.22)
project(DIPLConsumer NONE)
include("${SNT_HELPER}")
snt_dip_get(FILE config.dip PATH feature TYPE BOOL OUTPUT feature DEPENDS external.dip)
get_property(dependencies DIRECTORY PROPERTY CMAKE_CONFIGURE_DEPENDS)
if(NOT "${CMAKE_CURRENT_SOURCE_DIR}/external.dip" IN_LIST dependencies)
    message(FATAL_ERROR "Explicit dependency was not registered")
endif()
if(feature)
    set(branch enabled)
else()
    set(branch disabled)
endif()
file(WRITE "${CMAKE_BINARY_DIR}/branch.txt" "${branch}")
]=])
execute_process(COMMAND "${CMAKE_COMMAND}" -S "${source}" -B "${binary}" -G "${TEST_GENERATOR}"
    "-DSNT_EXECUTABLE=${SNT_EXECUTABLE}" "-DSNT_HELPER=${SNT_HELPER}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status STREQUAL "0")
    message(FATAL_ERROR "Consumer configure failed: ${output}\n${error}")
endif()
file(READ "${binary}/branch.txt" branch)
if(NOT branch STREQUAL "enabled")
    message(FATAL_ERROR "Initial DIPL branch was not enabled")
endif()
execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep 1)
file(WRITE "${source}/config.dip" "feature bool = false\n")
execute_process(COMMAND "${CMAKE_COMMAND}" --build "${binary}"
    RESULT_VARIABLE status OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT status STREQUAL "0")
    message(FATAL_ERROR "Consumer rebuild failed: ${output}\n${error}")
endif()
file(READ "${binary}/branch.txt" branch)
if(NOT branch STREQUAL "disabled")
    message(FATAL_ERROR "Editing DIPL did not regenerate the CMake branch")
endif()
