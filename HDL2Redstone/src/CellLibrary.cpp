#include <exception>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <CellLibrary.hpp>
#include <Exception.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

CellLibrary::CellLibrary(const std::string& CellLibDir_) {
    std::ifstream Json_in_fs(CellLibDir_ + "HDL2Redstone.json");
    if (Json_in_fs.fail()) {
        throw Exception("Failed reading json file: " + CellLibDir_ + "HDL2Redstone.json");
    }
    json J;
    Json_in_fs >> J;

    try {
        for (const auto& CellData : J) {
            std::map<std::string, std::map<std::string, std::string>> TempPinInfo;
            for (const auto& [key, value] : CellData["pins"].items()) {
                TempPinInfo.emplace(key, value);
            }
            std::string SchematicDir = CellLibDir_ + (CellData["is_component"] ? "components/" : "connections/");
            CellInstances.emplace(
                CellData["name"],
                std::make_unique<Cell>(CellData["name"], SchematicDir, std::move(TempPinInfo), CellData["schematic"]));
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
