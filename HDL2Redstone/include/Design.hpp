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
    // Create a design with BLIF file, cell library, and design constraint
    Design(const std::string& File_, const CellLibrary& CellLib_, const DesignConstraint& DC_, int Seed_);
    std::tuple<uint16_t, uint16_t, uint16_t> getSpace() const;
    const ModuleNetlist& getModuleNetlist() const;
    bool doPlaceAndRoute();

    Schematic exportDesign() const;

  private:
    uint16_t Width;
    uint16_t Height;
    uint16_t Length;
    const CellLibrary& CellLib;
    const DesignConstraint& DC;
    ModuleNetlist MN;
    std::map<std::string, Placement> ForcedPlacement;
    unsigned int Seed;

    friend class Placer;
    friend class Router;
    friend class Timing;
    friend std::ostream& operator<<(std::ostream& out, const Design& Design_);
};
} // namespace HDL2Redstone
