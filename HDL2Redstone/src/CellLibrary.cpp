#include <exception>
#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <CellLibrary.hpp>
#include <Exception.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

CellLibrary::CellLibrary(const std::string& Json_in_) {
    std::ifstream Json_in_fs(Json_in_);
    if (Json_in_fs.fail()) {
        throw Exception("Failed reading json file: " + Json_in_);
    }
    json J;
    Json_in_fs >> J;

    try {
        for (const auto& CellData : J) {
            std::map<std::string, std::map<std::string, std::string>> TempPinInfo;
            std::map<std::string, std::map<std::string, std::string>> TempSchemaInfo;
            for (const auto& [key, value] : CellData["pins"].items())
                TempPinInfo.emplace(key, value);
            for (const auto& [key, value] : CellData["schematics"].items()) {
                TempSchemaInfo.emplace(key, value);
            }
            CellInstances.emplace(CellData["name"], std::make_unique<Cell>(CellData["name"], std::move(TempPinInfo),
                                                                           std::move(TempSchemaInfo)));
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
