add_library(hdl2redstone-core
    Cell.cpp
    CellLibrary.cpp
    Component.cpp
    Connection.cpp
    Design.cpp
    Exception.cpp
    ModuleNetlist.cpp
    Placement.cpp
    Placer.cpp
    Router.cpp
    Schematic.cpp)
target_link_libraries(hdl2redstone-core libblifparse nbt++ nlohmann_json)
