find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(SERVER_TESTS PUQConvert DIPSingleValue DIPProjectBundle DIPSchemaUploads DIPH5 DIPCompare)
if(ENABLE_SNT_REPORT)
  list(APPEND SERVER_TESTS DIPReport)
endif()
foreach(TEST_NAME ${SERVER_TESTS})
  add_test(
    NAME server.${TEST_NAME}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/test_server.py
            ${TEST_NAME} $<TARGET_FILE:snt>
  )
  set_tests_properties(server.${TEST_NAME} PROPERTIES TIMEOUT 30)
endforeach()
