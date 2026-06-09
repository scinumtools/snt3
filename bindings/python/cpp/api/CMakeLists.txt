target_compile_definitions(${MODULE_NAME} PRIVATE ENABLE_API_PYBIND)
file(GLOB SOURCE_FILES "./*.cpp")    
target_sources(${MODULE_NAME} PRIVATE ${SOURCE_FILES})
target_link_libraries(${MODULE_NAME} PRIVATE snt-api)

target_include_directories(${MODULE_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

if (ENABLE_DIP_PYTEST)
    register_pytests(api)
endif()
