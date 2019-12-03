#include <Placer.hpp>

using namespace HDL2Redstone;

Placer::Placer(Design& D_) : D(D_) {}

bool Placer::place() {
    auto& Components = D.MN.getComponents();

    constexpr uint16_t Clearance = 10;

    uint16_t CurrentX = Clearance;
    uint16_t CurrentY = 0;
    uint16_t CurrentZ = Clearance;
    uint16_t MaxY = 0;
    uint16_t MaxZ = 0;

    // FIXME; Add more cells here
    std::set<std::string> AvailableCell{"INPUT", "OUTPUT", "NOT", "AND", "OR"};

    for (auto& Component : Components) {
        // Skip already placed component
        if (Component->getPlaced()) {
            continue;
        }

        if (AvailableCell.count(Component->getType())) {
            if (CurrentY + Component->getHeight() >= D.Height) {
                std::cout << "DEBUG: Placement error: not enough space!" << std::endl;
                return false;
            } else if (CurrentZ + Component->getLength() + Clearance >= D.Length) {
                CurrentX = Clearance;
                CurrentY += Clearance + MaxY;
                CurrentZ = Clearance;
                MaxY = 0;
                MaxZ = 0;
            } else if (CurrentX + Component->getWidth() + Clearance >= D.Width) {
                CurrentX = Clearance;
                CurrentZ += Clearance + MaxZ;
                MaxZ = 0;
            }
            // std::cout << CurrentX << " " << CurrentY << " " << CurrentZ << std::endl;
            Component->setPlacement(CurrentX, CurrentY, CurrentZ, Orientation::ZeroCW);
            CurrentX += Component->getWidth() + Clearance;
            if (Component->getLength() > MaxZ)
                MaxZ = Component->getLength();
            if (Component->getHeight() > MaxY)
                MaxY = Component->getHeight();
        }
    }
    return checkComponentLegality();
}

bool Placer::checkComponentLegality() const {
    std::vector<std::vector<std::vector<bool>>> OccupiedSpace(
        D.Width, std::vector<std::vector<bool>>(D.Height, std::vector<bool>(D.Length)));
    const auto& Components = D.MN.getComponents();

    for (const auto& Component : Components) {
        const auto& ComponentRange = Component->getRange();
        const auto& P1 = ComponentRange.first;
        const auto& P2 = ComponentRange.second;
        if (std::get<0>(P1) >= D.Width || std::get<1>(P1) >= D.Height || std::get<0>(P1) >= D.Length) {
            return false;
        }
        if (std::get<0>(P2) >= D.Width || std::get<1>(P2) >= D.Height || std::get<0>(P2) >= D.Length) {
            return false;
        }
        for (int X = std::get<0>(P1); X != std::get<0>(P2); ++X) {
            for (int Y = std::get<1>(P1); Y != std::get<1>(P2); ++Y) {
                for (int Z = std::get<2>(P1); Z != std::get<2>(P2); ++Z) {
                    if (OccupiedSpace[X][Y][Z]) {
                        return false;
                    } else {
                        OccupiedSpace[X][Y][Z] = true;
                    }
                }
            }
        }
    }
    return true;
}
