find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
include_directories(${Python3_INCLUDE_DIRS})
find_package(pybind11 CONFIG)

set(MODULE_NAME scinumtools3)

if (COMPILE_PUQ)
add_subdirectory(puq)
endif()
