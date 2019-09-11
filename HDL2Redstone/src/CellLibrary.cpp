#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <CellLibrary.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

CellLibrary::CellLibrary(const std::string& json_in_) {
    std::ifstream j_in(json_in_);
    if (j_in.fail()) {
        std::cout << "Error" << std::endl;
    }
    json j;
    j_in >> j;

    try {
        for (const auto& cell : j) {
            std::map<std::string, std::map<std::string, std::string>> temp;
            for (const auto& [key, value] : cell["pins"].items())
                temp.emplace(key, value);
            CellInstances.emplace(cell["name"], std::make_unique<Cell>(cell["name"], temp));
        }
    } catch (json::out_of_range& e) {
        std::cout << e.what() << std::endl;
    }
}
