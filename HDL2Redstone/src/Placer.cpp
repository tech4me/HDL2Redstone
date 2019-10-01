#include <Placer.hpp>

using namespace HDL2Redstone;

Placer::Placer(Design& _D) : D(_D) {}

bool Placer::place() {
    auto& Components = D.MN.getComponents();
    uint16_t size = Components.size();
    uint16_t unitX = D.Width / size;
    uint16_t unitY = D.Height / size;
    uint16_t unitZ = D.Length / size;

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
