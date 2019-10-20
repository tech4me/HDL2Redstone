#include <Component.hpp>

using namespace HDL2Redstone;
Component::Component(const Cell* CellPtr_) : CellPtr(CellPtr_), Placed(false) {}

Facing Component::getPinFacing(std::string PinName_) const {
    auto tempDir = CellPtr->getPinFacing(PinName_);
    if (tempDir == Facing::Up || tempDir == Facing::Down)
        return tempDir;
    if (P.Orient == Orientation::ZeroCW) {
        return tempDir;
    } else if (P.Orient == Orientation::OneCW) {
        if (tempDir == Facing::North)
            return Facing::East;
        if (tempDir == Facing::East)
            return Facing::South;
        if (tempDir == Facing::South)
            return Facing::West;
        if (tempDir == Facing::West)
            return Facing::North;
    } else if (P.Orient == Orientation::TwoCW) {
        if (tempDir == Facing::North)
            return Facing::South;
        if (tempDir == Facing::East)
            return Facing::West;
        if (tempDir == Facing::South)
            return Facing::North;
        if (tempDir == Facing::West)
            return Facing::East;
    } else {
        if (tempDir == Facing::North)
            return Facing::West;
        if (tempDir == Facing::East)
            return Facing::North;
        if (tempDir == Facing::South)
            return Facing::East;
        if (tempDir == Facing::West)
            return Facing::South;
    }
    return tempDir;
}
std::tuple<uint16_t, uint16_t, uint16_t> Component::getPinLocation(std::string PinName_) const {
    auto tempLoc = CellPtr->getPinLocation(PinName_);
    // Facing tempFac = Component::getPinFacing(PinName_);
    if (P.Orient == Orientation::ZeroCW) {
        return std::make_tuple(std::get<0>(tempLoc) + P.X, std::get<1>(tempLoc) + P.Y, std::get<2>(tempLoc) + P.Z);
    } else if (P.Orient == Orientation::OneCW) {
        return std::make_tuple(std::get<2>(tempLoc) + P.X, std::get<1>(tempLoc) + P.Y, -std::get<0>(tempLoc) + P.Z);
    } else if (P.Orient == Orientation::TwoCW) {
        return std::make_tuple(-std::get<0>(tempLoc) + P.X, std::get<1>(tempLoc) + P.Y, -std::get<2>(tempLoc) + P.Z);
    } else {
        return std::make_tuple(-std::get<2>(tempLoc) + P.X, std::get<1>(tempLoc) + P.Y, std::get<0>(tempLoc) + P.Z);
    }
}
void Component::setPlacement(uint16_t X_, uint16_t Y_, uint16_t Z_, Orientation Orient_) {
    P.X = X_;
    P.Y = Y_;
    P.Z = Z_;
    P.Orient = Orient_;
    Placed = true;
}

std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>
Component::getRange() const {
    uint16_t x1, y1, z1, x2, y2, z2;
    Schematic S = CellPtr->getSchematic();
    uint16_t Width = S.getWidth();
    uint16_t Height = S.getHeight();
    uint16_t Length = S.getLength();
    if (P.Orient == Orientation::ZeroCW) {
        x1 = P.X;
        y1 = P.Y;
        z1 = P.Z;
        x2 = P.X + Width;
        y2 = P.Y + Height;
        z2 = P.Z + Length;
    } else if (P.Orient == Orientation::OneCW) {
        x1 = P.X - Length;
        y1 = P.Y;
        z1 = P.Z;
        x2 = P.X;
        y2 = P.Y + Height;
        z2 = P.Z + Width;
    } else if (P.Orient == Orientation::TwoCW) {
        x1 = P.X - Width;
        y1 = P.Y;
        z1 = P.Z - Length;
        x2 = P.X;
        y2 = P.Y + Height;
        z2 = P.Z;
    } else {
        x1 = P.X;
        y1 = P.Y;
        z1 = P.Z - Width;
        x2 = P.X + Length;
        y2 = P.Y + Height;
        z2 = P.Z;
    }
    return std::pair(std::make_tuple(x1, y1, z1), std::make_tuple(x2, y2, z2));
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

        out << "   Location: " << Component_.P.X << " " << Component_.P.Y << " " << Component_.P.Z
            << "   Number of CW: " << CW << std::endl;
    } else {
        out << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
