# add submodule directories
if (ENABLE_PUQ)
   add_subdirectory(puq)
endif()
if (ENABLE_DIP) 
   add_subdirectory(dip) 
endif()
add_subdirectory(snt)
