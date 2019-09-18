#include <Component.hpp>

using namespace HDL2Redstone;
Component::Component(const Cell* CellPtr_) : CellPtr(CellPtr_), Placed(false) {}

void Component::setPlacement(const std::string& CellStructName_, uint16_t X_, uint16_t Y_, uint16_t Z_,
                             Orientation Orient_) {
    P.CellStructName = CellStructName_;
    P.X = X_;
    P.Y = Y_;
    P.Z = Z_;
    P.Orient = Orient_;
    Placed = true;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Component& Component_) {
    out << "Module Type: " << Component_.CellPtr->getType();
    if (Component_.Placed) {
        int CW;
        if (Component_.P.Orient == Orientation::ZeroCW) {
            CW = 0;
        } else if (Component_.P.Orient == Orientation::OneCW) {
            CW = 1;
        } else if (Component_.P.Orient == Orientation::TwoCW) {
            CW = 2;
        } else {
            CW = 3;
        }

        out << "   Cell Structure Name: " << Component_.P.CellStructName << "   Location: " << Component_.P.X << " "
            << Component_.P.Y << " " << Component_.P.Z << "   Number of CW: " << CW << std::endl;
    } else {
        out << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
