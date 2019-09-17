#include <Component.hpp>

using namespace HDL2Redstone;
Component::Component(const Cell* CellPtr_)
    : CellPtr(CellPtr_), Param(CellPtr_->getType()){
                             // TODO: replace gettype() with getcellinstance()
                         };

void Component::SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Orientation Turn_) {
    Param.SetParameters(CellStructure_, X_, Y_, Z_, Turn_);
}

Component::Parameters::Parameters(std::string CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Orientation Turn_)
    : CellStructure(CellStructure_), X(X_), Y(Y_), Z(Z_), Turn(Turn_) {}

void Component::Parameters::SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_,
                                          Orientation Turn_) {
    CellStructure = CellStructure_;
    X = X_;
    Y = Y_;
    Z = Z_;
    Turn = Turn_;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Component& Component_) {
    int CW;
    if (Component_.Param.Turn == Component::Orientation::ZeroCW) {
        CW = 0;
    } else if (Component_.Param.Turn == Component::Orientation::OneCW) {
        CW = 1;
    } else if (Component_.Param.Turn == Component::Orientation::TwoCW) {
        CW = 2;
    } else {
        CW = 3;
    }
    out << "Module Type: " << Component_.CellPtr->getType() << "   Cell Structure: " << Component_.Param.CellStructure
        << "   Location: " << Component_.Param.X << " " << Component_.Param.Y << " " << Component_.Param.Z
        << "   Number of CW: " << CW << std::endl;
    return out;
}
} // namespace HDL2Redstone
