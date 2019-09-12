#include <Parameters.hpp>

using namespace HDL2Redstone;
Parameters::Parameters() {} // Delete in the future
Parameters::Parameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Parameters::Orientation Turn_) {
    CellStructure = CellStructure_;
    X = X_;
    Y = Y_;
    Z = Z_;
    Turn = Turn_;
}
void Parameters::SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_,
                               Parameters::Orientation Turn_) {
    CellStructure = CellStructure_;
    X = X_;
    Y = Y_;
    Z = Z_;
    Turn = Turn_;
}
