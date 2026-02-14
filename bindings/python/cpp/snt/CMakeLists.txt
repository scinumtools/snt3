target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_SNT_PYBIND)
file(GLOB SOURCE_FILES "./*.cpp")    
target_sources(${MODULE_NAME} PRIVATE ${SOURCE_FILES})
