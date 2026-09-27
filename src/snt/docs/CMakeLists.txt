set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/template.tex")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/template.tex" SNT_DOCS_TEMPLATE)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/template.h.in" "${CMAKE_CURRENT_BINARY_DIR}/template.h" @ONLY)

add_library(snt-docs STATIC
  "${CMAKE_CURRENT_SOURCE_DIR}/model.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/writer.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/compiler.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/report.cpp"
)
target_link_libraries(snt-docs PUBLIC snt-dip)
target_include_directories(snt-docs
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:include>
  PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
set_target_properties(snt-docs PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
install(TARGETS snt-docs EXPORT TargetsSNT ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/snt/docs/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/snt/docs FILES_MATCHING PATTERN "*.h")
