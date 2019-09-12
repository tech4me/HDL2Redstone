#include <Component.hpp>

using namespace HDL2Redstone;
Component::Component(const Cell* CellPtr_)
    : CellPtr(CellPtr_){
          // TODO: intilized Param using CellPtr_
      };
// std::string Component::getCellType() const { return "HIGH"; };
void Component::SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_,
                              Parameters::Orientation Turn_) {
    Param.SetParameters(CellStructure_, X_, Y_, Z_, Turn_);
}
namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Component& Component_) {
    out << "Module Type:" << Component_.CellPtr->getType() << std::endl;
    return out;
}
} // namespace HDL2Redstone
