target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_PUQ_PYBIND)
file(GLOB SOURCE_FILES "./*.cpp")    
target_sources(${MODULE_NAME} PRIVATE ${SOURCE_FILES})
target_link_libraries(${MODULE_NAME} PRIVATE snt-puq)

if (ENABLE_PUQ_PYTEST)
    register_pytests(puq)
endif()
