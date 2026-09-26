# Add dimension-map generation to the main executable.
target_sources(snt PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)

target_include_directories(snt
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
)
