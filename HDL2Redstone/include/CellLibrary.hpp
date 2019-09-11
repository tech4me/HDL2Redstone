#pragma once

#include <string>

#include <Cell.hpp>
#include <Schematic.hpp>

namespace HDL2Redstone {
class CellLibrary {
  public:
    CellLibrary(const std::string& json_in);

  private:
    std::vector<Cell> cell_instances;
};
} // namespace HDL2Redstone
