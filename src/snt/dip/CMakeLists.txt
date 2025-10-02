# build library
file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-dip)

add_library(${EXEC_NAME} STATIC ${SOURCE_FILES})
target_compile_definitions(${EXEC_NAME} PRIVATE)
target_link_libraries(${EXEC_NAME} PRIVATE snt-puq)

if(NOT ENABLE_CLANG_TIDY)
target_precompile_headers(${EXEC_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../dip.h" INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/../dip.h")
endif()

set_target_properties(${EXEC_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/${MODULE_NAME})
