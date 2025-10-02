# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-val)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${EXEC_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

if(NOT ENABLE_CLANG_TIDY)
target_precompile_headers(${EXEC_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../val.h" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../val.h")
endif()

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MODULE_NAME})
