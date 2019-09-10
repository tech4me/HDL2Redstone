#pragma once

#include <string>

#include <Cell.hpp>

namespace HDL2Redstone {
class Component {
  public:
    Component(const std::string& TypeName_);
    Component(Cell* CellPtr_) : CellPtr(CellPtr_) {};
    // Port* getPort(std::string PortName);
    std::string getType() const;

  private:
    const Cell* CellPtr;
};
} // namespace HDL2Redstone
