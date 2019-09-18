#include <Cell.hpp>
#include <Exception.hpp>

using namespace HDL2Redstone;

Cell::PinInfo::PinInfo(std::map<std::string, std::string> JsonInfo_) {
    const auto& Direction = JsonInfo_["direction"];
    if (Direction == "input") {
        Dir = Direction::Input;
    } else if (Direction == "output") {
        Dir = Direction::Output;
    } else if (Direction == "inout") {
        Dir = Direction::Inout;
    } else {
        throw Exception("Invalid cell pin direction: " + Direction + " .");
    }
}

Cell::SchemaInfo::SchemaInfo(const std::string& SchemaPath_, const std::map<std::string, std::string>& JsonInfo_)
    : Schem(SchemaPath_) {
    /*const auto& Orientation = JsonInfo_["orientation"];
     *if (Orientation == "north") {
        Ori = North;
     }*/
}

Cell::Cell(const std::string& Type_, const std::string& CellLibDir_,
           const std::map<std::string, std::map<std::string, std::string>>& Pins_,
           const std::map<std::string, std::map<std::string, std::string>>& Schematics_)
    : Type(Type_) {
    for (const auto& Pin : Pins_) {
        Pins.emplace(Pin.first, PinInfo(Pin.second));
    }
    for (const auto& Schem : Schematics_) {
        std::string SchPath = CellLibDir_ + Type_ + "/" + Schem.first + ".schem";
        Schematics.emplace(Schem.first, SchemaInfo(SchPath, Schem.second));
    }
}
