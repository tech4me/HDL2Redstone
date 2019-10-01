#include <Placer.hpp>

using namespace HDL2Redstone;
bool Placer::placing(ModuleNetlist& MN, uint16_t Width, uint16_t Height, uint16_t Length) {
    auto& Components = MN.getComponents();
    uint16_t size = Components.size();
    uint16_t unitX = Width / size;
    uint16_t unitY = Height / size;
    uint16_t unitZ = Length / size;

    int16_t i = 0;
    for (auto& Component : Components) {
        // FIXME: We only place the gates we have for now
        if (Component->getType() == "INPUT") {
            Component->setPlacement("input", i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "OUTPUT") {
            Component->setPlacement("output", i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "NOT") {
            Component->setPlacement("horizontal_torch_inv", i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        } else if (Component->getType() == "AND") {
            Component->setPlacement("basic_and_gate", i * unitX, i * unitY, i * unitZ, Orientation::OneCW);
            ++i;
        }
    }
    return true;
}