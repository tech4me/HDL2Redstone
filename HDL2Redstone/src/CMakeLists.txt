add_library(hdl2redstone-core
    Cell.cpp
    CellLibrary.cpp
    Component.cpp
    Connection.cpp
    Design.cpp
    ModuleNetlist.cpp
    Parameters.cpp
    Schematic.cpp)
target_link_libraries(hdl2redstone-core libblifparse nbt++ nlohmann_json)
