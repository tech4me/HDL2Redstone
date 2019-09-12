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
    : Schema(SchemaPath_) {
    /*const auto& Orientation = JsonInfo_["orientation"];
     *if (Orientation == "north") {
        Ori = North;
     }*/
}

Cell::Cell(const std::string& Type_, const std::map<std::string, std::map<std::string, std::string>>& Pins_,
           const std::map<std::string, std::map<std::string, std::string>>& Schematics_)
    : Type(Type_) {
    for (const auto& Pin : Pins_) {
        Pins.emplace(Pin.first, PinInfo(Pin.second));
    }
    for (const auto& Schema : Schematics_) {
        std::string SchPath = "../HDL2Redstone/cell_lib/" + Type_ + "/" + Schema.first + ".schematic";
        Schematics.emplace(Type_, SchemaInfo(SchPath, Schema.second));
    }
    // Schematic S;
    // S.loadSchematic("../HDL2Redstone/cell_lib/<cell type>/<sch name>.schematic")
}

const std::string& Cell::getType() const { return Type; }
