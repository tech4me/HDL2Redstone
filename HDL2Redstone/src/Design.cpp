#include <Design.hpp>
#include <Placer.hpp>
using namespace HDL2Redstone;

Design::Design(uint16_t Width_, uint16_t Height_, uint16_t Length_, const std::string& File_,
               const CellLibrary& CellLib_)
    : Width(Width_), Height(Height_), Length(Length_), MN(File_, CellLib_) {}

bool Design::place() {
    Placer P;
    P.placing(MN, Width, Height, Length);
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
