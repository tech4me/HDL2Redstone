#include <Design.hpp>

using namespace HDL2Redstone;
Design::Design(int16_t Width, int16_t Height, int16_t Length, const std::string& File_, const CellLibrary& CellLib_)
    : X(Width), Y(Length), Z(Height), MN(File_, CellLib_) {}

bool Design::place() {
    std::vector<std::unique_ptr<Component>>* ComponentPtr_;
    ComponentPtr_ = MN.getComponentsPtr();
    int16_t size = ComponentPtr_->size();
    int16_t unitX = X / size;
    int16_t unitY = Y / size;
    int16_t unitZ = Z / size;

    int16_t i = 0;
    std::string temp = "N/A";
    for (auto& T : *ComponentPtr_) {
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