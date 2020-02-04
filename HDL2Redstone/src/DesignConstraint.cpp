#include <fstream>
#include <iostream>

#include <nlohmann/json.hpp>

#include <DesignConstraint.hpp>
#include <Exception.hpp>

using namespace nlohmann;
using namespace HDL2Redstone;

namespace HDL2Redstone {
NLOHMANN_JSON_SERIALIZE_ENUM(Orientation, {{Orientation::ZeroCW, 0},
                                           {Orientation::OneCW, 1},
                                           {Orientation::TwoCW, 2},
                                           {Orientation::ThreeCW, 3}})
} // namespace HDL2Redstone

bool DesignConstraint::readDCFile(const std::string& File_) {
    std::ifstream JSON(File_);
    if (JSON.fail()) {
        std::cout << "Error parsing: " << File_ << std::endl;
        return true;
    }

    json J;
    JSON >> J;

    try {
        Dimension = std::make_tuple(J["dimension"][0], J["dimension"][1], J["dimension"][2]);
        for (const auto& [K, V] : J["forced_placement"].items()) {
            addForcedPlacement(
                K, {V["location"][0], V["location"][1], V["location"][2], V["orientation"].get<Orientation>()});
        }
    } catch (json::exception& E) {
        throw Exception(E.what());
    }
    return false;
}
