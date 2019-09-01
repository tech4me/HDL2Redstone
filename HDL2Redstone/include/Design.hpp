#pragma once

#include <Schematic.hpp>

class Design {
  public:
    // Deleted default constructor
    Design() = delete;
    // Create a design with max width, max height, and max length
    Design(int16_t Width, int16_t Height, int16_t Length);

    // Add a Schematic to the Design
    //bool addSchematic(Schematic Sch, int16_t X, int16_t Y, int16_t Z);

    Schematic exportDesign();
  private:
};
