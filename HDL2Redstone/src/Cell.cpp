#include <Cell.hpp>
#include <Exception.hpp>

using namespace HDL2Redstone;

Cell::Cell(const std::string& Type_, const std::string& CellLibDir_,
           const std::map<std::string, std::map<std::string, std::string>>& Pins_, const std::string& SchematicName_)
    : Type(Type_), Schem(CellLibDir_ + SchematicName_ + ".schem") {
    for (const auto& Pin : Pins_) {
        Pins.emplace(Pin.first, PinInfo(Pin.second));
    }
}

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

    const auto& Facing = JsonInfo_["facing"];
    if (Facing == "up") {
        Face = Facing::Up;
    } else if (Facing == "down") {
        Face = Facing::Down;
    } else if (Facing == "north") {
        Face = Facing::North;
    } else if (Facing == "south") {
        Face = Facing::South;
    } else if (Facing == "east") {
        Face = Facing::East;
    } else if (Facing == "west") {
        Face = Facing::West;
    } else {
        // TODO: Uncomment below once we have facing info
        // throw Exception("Invalid cell pin facing: " + Facing + " .");
    }
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Cell& Cell_) {
    out << "In cell " << Cell_.Type << ": " << std::endl;

    // Dir is in enum class, will add print if needed, right now not gonna print
    /*out << "Pins: " << std::endl;
    for (const auto& Pin : Cell_.Pins) {
        out << Pin.first << ": " << std::endl;
        out << "Direction: " << Pin.second.Dir << std::endl;
    }*/

    out << "Schematic: " << std::endl;
    out << Cell_.Schem << ": " << std::endl;

    return out;
}

} // namespace HDL2Redstone
