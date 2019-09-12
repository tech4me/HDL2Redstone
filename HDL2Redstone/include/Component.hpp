#pragma once

#include <iostream>
#include <string>

#include <Cell.hpp>
#include <Parameters.hpp>
namespace HDL2Redstone {
class Component {
  public:
    Component(const Cell* CellPtr_);
    void SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Parameters::Orientation Turn_);
    friend std::ostream& operator<<(std::ostream& out, const Component& Component_);

  private:
    const Cell* CellPtr;
    Parameters Param;
};
} // namespace HDL2Redstone
