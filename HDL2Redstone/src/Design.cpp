#include <Design.hpp>

using namespace HDL2Redstone;
Design::Design(int16_t Width_, int16_t Height_, int16_t Length_, const std::string& File_, const CellLibrary& CellLib_)
    : Width(Width_), Height(Height_), Length(Length_), MN(File_, CellLib_) {}

bool Design::place() {
    auto& Component = MN.getComponents();
    int16_t size = Component.size();
    int16_t unitX = Width / size;
    int16_t unitY = Height / size;
    int16_t unitZ = Length / size;

    int16_t i = 0;
    std::string temp = "N/A";
    for (auto& T : Component) {
        T->SetParameters(temp, i * unitX, i * unitY, i * unitZ, Component::Orientation::OneCW);
        i++;
    }

    return true;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Design& Design_) {
    out << "Design Layout:"
        << "\n"
        << Design_.MN;
    return out;
}
} // namespace HDL2Redstone
