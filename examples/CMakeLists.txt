# add submodule directories
if (ENABLE_EXS)
   add_subdirectory(exs)
endif()
if (ENABLE_DIP)
   add_subdirectory(dip/AdapterOutputs)
   add_subdirectory(dip/InspectionGraph)
endif()
