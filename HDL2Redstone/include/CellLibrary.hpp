#pragma once

#include <map>
#include <memory>
#include <string>

#include <Cell.hpp>
#include <Schematic.hpp>

namespace HDL2Redstone {
class CellLibrary {
  public:
    CellLibrary(const std::string& CellLibDir_);
    const Cell* getCellPtr(const std::string& Name_) const;

  private:
    std::map<std::string, std::unique_ptr<Cell>> CellInstances;

    friend std::ostream& operator<<(std::ostream& out, const CellLibrary& CellLibrary_);
};
} // namespace HDL2Redstone
