file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
target_sources(${MODULE_NAME} PRIVATE ${SOURCE_FILES})
target_link_libraries(${MODULE_NAME} PRIVATE snt-exs)
   
