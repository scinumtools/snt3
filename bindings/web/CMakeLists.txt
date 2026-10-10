cmake_minimum_required(VERSION 3.22)
project(snt_dipl_web LANGUAGES CXX)

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Build this integration with emcmake cmake -S bindings/web -B build-web")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Emscripten renamed TOTAL_STACK to STACK_SIZE. Probe the linker setting so
# distribution-packaged and current SDK toolchains both get the same stack.
include(CheckLinkerFlag)
check_linker_flag(CXX "-sSTACK_SIZE=4194304" SNT_WEB_HAS_STACK_SIZE)
if(SNT_WEB_HAS_STACK_SIZE)
  set(SNT_WEB_STACK_OPTION "-sSTACK_SIZE=4194304")
else()
  check_linker_flag(CXX "-sTOTAL_STACK=4194304" SNT_WEB_HAS_TOTAL_STACK)
  if(NOT SNT_WEB_HAS_TOTAL_STACK)
    message(FATAL_ERROR "Emscripten does not support a recognized stack-size setting")
  endif()
  set(SNT_WEB_STACK_OPTION "-sTOTAL_STACK=4194304")
endif()

# Compile the existing language implementation without native persistence,
# reports, adapters, or application front ends. No browser parser is forked.
set(SNT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
file(GLOB_RECURSE SNT_SOURCES CONFIGURE_DEPENDS
  "${SNT_ROOT}/src/snt/core/*.cpp"
  "${SNT_ROOT}/src/snt/exs/*.cpp"
  "${SNT_ROOT}/src/snt/val/*.cpp"
  "${SNT_ROOT}/src/snt/puq/*.cpp"
  "${SNT_ROOT}/src/snt/dip/*.cpp")
list(FILTER SNT_SOURCES EXCLUDE REGEX "/(hdf5|report|adapter|generate)/")
list(FILTER SNT_SOURCES EXCLUDE REGEX "/src/snt/dip/(artifact|comparison|preview)\\.cpp$")

add_executable(dipl_web embind.cpp runtime.cpp "${SNT_ROOT}/src/snt/api/puq_convert.cpp" ${SNT_SOURCES})
target_include_directories(dipl_web PRIVATE "${SNT_ROOT}/include" "${SNT_ROOT}/src")
target_compile_options(dipl_web PRIVATE -fexceptions)
target_link_options(dipl_web PRIVATE
  --bind
  -fexceptions
  "-sMODULARIZE=1"
  "-sEXPORT_ES6=1"
  "-sEXPORT_NAME=createDIPLRuntime"
  "-sENVIRONMENT=web,worker,node"
  "-sALLOW_MEMORY_GROWTH=1"
  "-sINITIAL_MEMORY=33554432"
  "${SNT_WEB_STACK_OPTION}"
  "-sSTACK_OVERFLOW_CHECK=2")
set_target_properties(dipl_web PROPERTIES
  SUFFIX ".mjs"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/dist")
