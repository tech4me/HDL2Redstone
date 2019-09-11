#pragma once

#include <iostream>
#include <string>

#include <Cell.hpp>

namespace HDL2Redstone {
class Component {
  public:
    Component(const Cell* CellPtr_);
    std::string getCellType() const;
    friend std::ostream& operator<<(std::ostream& out, const Component& Component_);

  private:
    const Cell* CellPtr;
};
} // namespace HDL2Redstone
