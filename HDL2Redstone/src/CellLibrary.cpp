#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <CellLibrary.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

CellLibrary::CellLibrary(const std::string& json_in_) {
    std::ifstream j_in(json_in_);
    json j;
    j_in >> j;
    std::cout<<j;
}
