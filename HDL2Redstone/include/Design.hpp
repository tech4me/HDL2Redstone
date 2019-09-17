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
    Design(int16_t Width, int16_t Height, int16_t Length, const std::string& File_, const CellLibrary& CellLib_);

    bool place();
    bool route();

    // Add a Schematic to the Design
    // bool addSchematic(Schematic Sch, int16_t X, int16_t Y, int16_t Z);

    Schematic exportDesign();

  private:
    int16_t X, Y, Z;
    ModuleNetlist MN;
    friend std::ostream& operator<<(std::ostream& out, const Design& Design_);
};
} // namespace HDL2Redstone
