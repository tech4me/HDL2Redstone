add_library(hdl2redstone-core
    Component.cpp
    Connection.cpp
    Design.cpp
    ModuleNetlist.cpp
    Schematic.cpp)
target_link_libraries(hdl2redstone-core libblifparse nbt++)
