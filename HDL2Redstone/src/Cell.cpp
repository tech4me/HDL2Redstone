#include <Cell.hpp>
#include <Exception.hpp>

using namespace HDL2Redstone;

Cell::Cell(const std::string& Type_, const std::string& CellLibDir_, const std::map<std::string, Pin> Pins_, int Delay_,
           const std::string& SchematicName_)
    : Type(Type_), Schem(CellLibDir_ + SchematicName_ + ".schem"), Pins(Pins_), Delay(Delay_) {}

std::vector<std::string> Cell::getPinNames() const {
    std::vector<std::string> Ret;
    for (const auto& Pin : Pins) {
        Ret.push_back(Pin.first);
    }
    return Ret;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Cell& Cell_) {
    out << "In cell " << Cell_.Type << ": " << std::endl;

    out << "Pins: " << std::endl;
    for (const auto& Pin : Cell_.Pins) {
        out << "  " << Pin.first << ": " << std::endl;
        std::string dir, face;
        switch (static_cast<int>(Pin.second.Dir)) {
        case 0:
            dir = "input";
            break;
        case 1:
            dir = "output";
            break;
        case 2:
            dir = "inout";
            break;
        }
        switch (static_cast<int>(Pin.second.Face)) {
        case 0:
            face = "up";
            break;
        case 1:
            face = "down";
            break;
        case 2:
            face = "north";
            break;
        case 3:
            face = "south";
            break;
        case 4:
            face = "east";
            break;
        case 5:
            face = "west";
            break;
        }
        out << "    Direction: " << dir << "; Facing: " << face << "; Location: " << std::get<0>(Pin.second.Location)
            << " " << std::get<1>(Pin.second.Location) << " " << std::get<2>(Pin.second.Location) << std::endl;
    }

    out << "Schematic: " << std::endl;
    out << Cell_.Schem << ": " << std::endl;

    return out;
}

} // namespace HDL2Redstone
