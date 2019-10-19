#include <Placer.hpp>

using namespace HDL2Redstone;

Placer::Placer(Design& D_) : D(D_) {}

bool Placer::place() {
    auto& Components = D.MN.getComponents();
    uint16_t size = Components.size();
    uint16_t unitX = D.Width / size;
    uint16_t unitY = D.Height / size;
    uint16_t unitZ = D.Length / size;

    int16_t i = 0;
    for (auto& Component : Components) {
        if (Component->getPlaced()) {
            continue;
        }

        // FIXME: We only place the gates we have for now
        if (Component->getType() == "INPUT") {
            Component->setPlacement(i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "OUTPUT") {
            Component->setPlacement(i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "NOT") {
            Component->setPlacement(i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "AND") {
            Component->setPlacement(3, 0, 3, Orientation::OneCW);
            //Component->setPlacement(i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
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
        for (int X = std::get<0>(P1); X != std::get<0>(P2); ++X) {
            for (int Y = std::get<1>(P1); Y != std::get<1>(P2); ++Y) {
                for (int Z = std::get<1>(P1); Z != std::get<1>(P2); ++Z) {
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
