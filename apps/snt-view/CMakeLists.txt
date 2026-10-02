set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/glfw/CMakeLists.txt" OR
   NOT EXISTS "${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp")
  message(FATAL_ERROR "snt view requires the imgui and glfw submodules. Run git submodule update --init external/imgui external/glfw.")
endif()

if(UNIX AND NOT APPLE AND (NOT DEFINED CACHE{GLFW_BUILD_X11} OR GLFW_BUILD_X11))
  find_package(X11 QUIET)
  if(NOT X11_FOUND)
    set(GLFW_BUILD_X11 OFF CACHE BOOL "Build GLFW with X11 support" FORCE)
    message(STATUS "X11 development library unavailable; building GLFW without X11 support")
  endif()
endif()

add_subdirectory("${PROJECT_SOURCE_DIR}/external/glfw" "${CMAKE_CURRENT_BINARY_DIR}/glfw")
if(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(SNT_VIEW_GL REQUIRED IMPORTED_TARGET gl)
  set(SNT_VIEW_OPENGL_TARGET PkgConfig::SNT_VIEW_GL)
else()
  find_package(OpenGL REQUIRED)
  set(SNT_VIEW_OPENGL_TARGET OpenGL::GL)
endif()

add_library(snt-view-imgui STATIC
  "${PROJECT_SOURCE_DIR}/external/imgui/imgui.cpp"
  "${PROJECT_SOURCE_DIR}/external/imgui/imgui_draw.cpp"
  "${PROJECT_SOURCE_DIR}/external/imgui/imgui_tables.cpp"
  "${PROJECT_SOURCE_DIR}/external/imgui/imgui_widgets.cpp"
  "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp"
  "${PROJECT_SOURCE_DIR}/external/imgui/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(snt-view-imgui PUBLIC
  "${PROJECT_SOURCE_DIR}/external/imgui"
  "${PROJECT_SOURCE_DIR}/external/imgui/backends")
target_link_libraries(snt-view-imgui PUBLIC glfw ${SNT_VIEW_OPENGL_TARGET})

add_subdirectory("${PROJECT_SOURCE_DIR}/docs/dipl/highlight/cpp"
                 "${CMAKE_CURRENT_BINARY_DIR}/dipl-highlighter")

target_sources(snt PRIVATE
  "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/application.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/source_view.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/viewer_model.cpp")
target_link_libraries(snt PRIVATE snt-view-imgui snt-dipl-highlighter)

if(APPLE)
  # The snt CLI is not an .app bundle, so embed the macOS Retina capability.
  target_link_options(snt PRIVATE
    "LINKER:-sectcreate,__TEXT,__info_plist,${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
endif()

if(ENABLE_UNIT_TESTS)
  add_executable(snt-view-model-test
    "${CMAKE_CURRENT_SOURCE_DIR}/viewer_model.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source_view.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/model.cpp")
  target_include_directories(snt-view-model-test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
  target_compile_definitions(snt-view-model-test PRIVATE PROJECT_SOURCE_ROOT_DIR="${PROJECT_SOURCE_DIR}/")
  target_link_libraries(snt-view-model-test PRIVATE snt-dip snt-dipl-highlighter)
  add_test(NAME view.model COMMAND snt-view-model-test)
endif()
