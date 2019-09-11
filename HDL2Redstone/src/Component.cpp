#include <Component.hpp>

using namespace HDL2Redstone;
Component::Component(const Cell* CellPtr_) : CellPtr(CellPtr_){};
std::string Component::getCellType() const { return "HIGH"; };

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Component& Component_) {
    out << "Module Type:" << Component_.getCellType() << std::endl;
    return out;
}
}