# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-exs)

add_library(${EXEC_NAME} INTERFACE)

if(NOT ENABLE_CLANG_TIDY)
target_precompile_headers(${EXEC_NAME} INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../exs.h")
endif()

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MODULE_NAME})
