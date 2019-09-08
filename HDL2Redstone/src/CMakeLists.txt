add_library(hdl2redstone-core
    ComponentNetlist.cpp
    Design.cpp
    Schematic.cpp)
target_link_libraries(hdl2redstone-core libblifparse nbt++)
