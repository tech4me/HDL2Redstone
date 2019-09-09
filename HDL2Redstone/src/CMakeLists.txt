add_library(hdl2redstone-core
    Design.cpp
    ModuleNetlist.cpp
	ModulePort.cpp
    Schematic.cpp)
target_link_libraries(hdl2redstone-core libblifparse nbt++)
