#include <Design.hpp>

using namespace HDL2Redstone;

Design::Design(uint16_t Width_, uint16_t Height_, uint16_t Length_, const std::string& File_,
               const CellLibrary& CellLib_)
    : Width(Width_), Height(Height_), Length(Length_), MN(File_, CellLib_) {}

bool Design::place() {
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

Schematic Design::exportDesign() const {
    Schematic Schem(Width, Height, Length);
    for (const auto& Component : MN.getComponents()) {
        if (Component->getPlaced()) {
            Schem.insertSubSchematic(Component->getPlacement(), Component->getSchematic());
        }
    }
    return Schem;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Design& Design_) {
    out << "Design Layout:"
        << "\n"
        << Design_.MN;
    return out;
}
} // namespace HDL2Redstone
