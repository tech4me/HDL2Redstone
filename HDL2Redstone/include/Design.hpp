#pragma once

#include <iostream>

#include <DesignConstraint.hpp>
#include <ModuleNetlist.hpp>
#include <Placement.hpp>
#include <Schematic.hpp>

namespace HDL2Redstone {
class Design {
  public:
    // Deleted default constructor
    Design() = delete;
    // Create a design with max width, max height, and max length
    // Verilog file, cell library, and design constraint
    Design(uint16_t Width, uint16_t Height, uint16_t Length, const std::string& File_, const CellLibrary& CellLib_,
           const DesignConstraint& DC_);
    const std::tuple<uint16_t, uint16_t, uint16_t> getSpace() const;
    const ModuleNetlist& getModuleNetlist() const;
    bool doPlaceAndRoute();

    Schematic exportDesign() const;

  private:
    uint16_t Width;
    uint16_t Height;
    uint16_t Length;
    const DesignConstraint& DC;
    ModuleNetlist MN;
    std::map<std::string, Placement> ForcedPlacement;

    friend class Placer;
    friend class Router;
    friend std::ostream& operator<<(std::ostream& out, const Design& Design_);
};
} // namespace HDL2Redstone
