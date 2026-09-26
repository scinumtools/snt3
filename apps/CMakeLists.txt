# add submodule directories
if (ENABLE_PUQ AND ENABLE_DIP AND ENABLE_API AND ENABLE_EXEC_APPS_SNT)
   add_subdirectory(snt)
endif()
if (ENABLE_PUQ AND ENABLE_EXEC_APPS_DMAP)
   add_subdirectory(snt-dmap)
endif()
