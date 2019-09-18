#pragma once

#include <iostream>

#include <ModuleNetlist.hpp>
#include <Schematic.hpp>

namespace HDL2Redstone {
class Design {
  public:
    // Deleted default constructor
    Design() = delete;
    // Create a design with max width, max height, and max length
    Design(uint16_t Width, uint16_t Height, uint16_t Length, const std::string& File_, const CellLibrary& CellLib_);

    bool place();
    bool route();

    Schematic exportDesign() const;

  private:
    uint16_t Width;
    uint16_t Height;
    uint16_t Length;
    ModuleNetlist MN;
    friend std::ostream& operator<<(std::ostream& out, const Design& Design_);
};
} // namespace HDL2Redstone
