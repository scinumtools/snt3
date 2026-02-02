file(GLOB SOURCE_FILES "./*.cpp" "./**/*.cpp")
target_sources(${MODULE_NAME} PRIVATE ${SOURCE_FILES})
