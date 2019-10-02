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
std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>
Component::getRange() const {
    uint16_t x1, z1, y1, x2, z2, y2;
    Schematic S = CellPtr->getSchematic(P.CellStructName);
    uint16_t Width = S.getWidth();
    uint16_t Height = S.getHeight();
    uint16_t Length = S.getLength();
    if (P.Orient == Orientation::ZeroCW) {
        x1 = P.X;
        z1 = P.Z;
        y1 = P.Y;
        x2 = P.X + Width;
        z2 = P.Z + Length;
        y2 = P.Y + Height;
    } else if (P.Orient == Orientation::OneCW) {
        x1 = P.X;
        z1 = P.Z - Width;
        y1 = P.Y;
        x2 = P.X + Length;
        z2 = P.Z;
        y2 = P.Y + Height;
    } else if (P.Orient == Orientation::TwoCW) {
        x1 = P.X - Width;
        z1 = P.Z - Length;
        y1 = P.Y;
        x2 = P.X;
        z2 = P.Z;
        y2 = P.Y + Height;
    } else {
        x1 = P.X - Length;
        z1 = P.Z;
        y1 = P.Y;
        x2 = P.X;
        z2 = P.Z + Width;
        y2 = P.Y + Height;
    }
    return std::pair(std::make_tuple(x1, z1, y1), std::make_tuple(x2, z2, y2));
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
