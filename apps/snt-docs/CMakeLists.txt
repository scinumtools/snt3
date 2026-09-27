file(READ "${CMAKE_CURRENT_SOURCE_DIR}/template.tex" SNT_DOCS_TEMPLATE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/template.h.in" "${CMAKE_CURRENT_BINARY_DIR}/template.h" @ONLY)

target_sources(snt PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/writer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/module.cpp"
)
target_include_directories(snt PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")

if(ENABLE_UNIT_TESTS)
  add_test(NAME cli.docs
    COMMAND ${CMAKE_COMMAND}
      "-DSNT_EXECUTABLE=$<TARGET_FILE:snt>"
      "-DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/tests"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/docs.cmake")
endif()
