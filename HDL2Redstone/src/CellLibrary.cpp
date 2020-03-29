#include <exception>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <CellLibrary.hpp>
#include <Exception.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

namespace HDL2Redstone {

NLOHMANN_JSON_SERIALIZE_ENUM(Direction,
                             {{Direction::Input, "input"}, {Direction::Output, "output"}, {Direction::Inout, "inout"}})
NLOHMANN_JSON_SERIALIZE_ENUM(Facing, {{Facing::Up, "up"},
                                      {Facing::Down, "down"},
                                      {Facing::North, "north"},
                                      {Facing::South, "south"},
                                      {Facing::East, "east"},
                                      {Facing::West, "west"}})

void from_json(const json& J_, Pin& P_) {
    P_.Dir = J_.at("direction").get<Direction>();
    P_.Face = J_.at("facing").get<Facing>();
    J_.at("location").get_to(P_.Location);
    P_.Power = J_.at("power").get<int>();
}

} // namespace HDL2Redstone

CellLibrary::CellLibrary(const std::string& CellLibDir_) {
    std::ifstream Json_in_fs(CellLibDir_ + "HDL2Redstone.json");
    if (Json_in_fs.fail()) {
        throw Exception("Failed reading json file: " + CellLibDir_ + "HDL2Redstone.json");
    }
    json J;
    Json_in_fs >> J;

    try {
        for (const auto& CellData : J) {
            std::map<std::string, Pin> TempPins;
            for (const auto& [key, value] : CellData.at("pins").items()) {
                TempPins.emplace(key, value.get<HDL2Redstone::Pin>());
            }
            std::string SchematicDir = CellLibDir_ + (CellData.at("is_component") ? "components/" : "connections/");
            CellInstances.emplace(
                CellData.at("name"),
                std::make_unique<Cell>(CellData.at("name"), SchematicDir, std::move(TempPins),
                                       CellData.contains("delay") ? CellData.at("delay").get<int>() : 0,
                                       CellData.at("schematic")));
        }
    } catch (json::exception& E) {
        throw Exception(E.what());
    }
}

const Cell* CellLibrary::getCellPtr(const std::string& Name_) const {
    try {
        return CellInstances.at(Name_).get();
    } catch (std::out_of_range& E) {
        throw Exception("Invalid cell type: " + Name_);
    }
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const CellLibrary& CellLibrary_) {
    for (const auto& Inst : CellLibrary_.CellInstances) {
        out << *Inst.second << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
