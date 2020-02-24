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

    uint16_t port_size = 1; // assuming it's 1 now

    try {
        Dimension = std::make_tuple(J["dimension"][0], J["dimension"][1], J["dimension"][2]);
        for (const auto& [K, V] : J["forced_placement"].items()) {
            // check if port location is on the boundary of the specified dimension
            bool x = V["location"][0] == 0 ? 1 : (V["location"][0] == std::get<0>(Dimension) - 1 ? 1 : 0);
            bool y = V["location"][1] == 0 ? 1 : (V["location"][1] == std::get<1>(Dimension) - 2 ? 1 : 0);
            bool z = V["location"][2] == 0 ? 1 : (V["location"][2] == std::get<2>(Dimension) - 1 ? 1 : 0);

            if (x & y || x & z || y & z) {
                uint16_t new_x = V["location"][0];
                uint16_t new_y = V["location"][1];
                uint16_t new_z = V["location"][2];
                Orientation ori = Orientation::ZeroCW;
                // adjust port facing based on where user wants to put it, taking into consideration there's a sign
                // sign will be at user specified location by default, facing west; input port will face east
                if (V["location"][0] == 0) { // port facing east
                    ori = Orientation::ZeroCW;
                } else if (V["location"][0] == std::get<0>(Dimension) - 1) { // w
                    ori = Orientation::TwoCW;
                    new_z += port_size;
                    new_x += port_size;
                } else if (V["location"][2] == 0) { // s
                    ori = Orientation::OneCW;
                    new_x += port_size;
                } else if (V["location"][2] == std::get<2>(Dimension) - 1) { // n
                    ori = Orientation::ThreeCW;
                    new_z += port_size;
                }
                // std::cout<<"K: "<<K<<", ori: "<<(int)ori<<std::endl;
                addForcedPlacement(
                    K, {/*V["location"][0], V["location"][1], V["location"][2], V["orientation"].get<Orientation>()*/
                        new_x, new_y, new_z, ori});
            } else {
                std::cout << "Port placement too interior, consider changing your port location selection!"
                          << std::endl;
                return true;
            }
        }
    } catch (json::exception& E) {
        throw Exception(E.what());
    }
    return false;
}
