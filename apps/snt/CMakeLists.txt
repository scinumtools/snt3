# create library
file(GLOB EXEC_FILES "./*.cpp" "./**/*.cpp")
set(EXEC_NAME snt-cli)
add_library(${EXEC_NAME} ${EXEC_FILES})

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_link_libraries(${EXEC_NAME} PRIVATE snt-puq snt-dip)

# create executable
set(EXEC_NAME snt)

add_executable(${EXEC_NAME} main.cpp )

target_include_directories(${EXEC_NAME}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)

target_link_libraries(${EXEC_NAME} PRIVATE snt-cli)

set_target_properties(${EXEC_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# install libraries and executables
install(TARGETS ${EXEC_NAME} EXPORT TargetsSNT
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
