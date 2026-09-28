find_package(Python3 REQUIRED COMPONENTS Interpreter)

foreach(TEST_NAME PUQConvert DIPSingleValue DIPProjectBundle DIPSchemaUploads DIPH5 DIPReport)
  add_test(
    NAME server.${TEST_NAME}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_server.py
            ${TEST_NAME} $<TARGET_FILE:snt>
  )
  set_tests_properties(server.${TEST_NAME} PROPERTIES TIMEOUT 30)
endforeach()
