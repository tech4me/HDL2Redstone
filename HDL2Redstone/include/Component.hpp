#pragma once

#include <iostream>
#include <string>

#include <Cell.hpp>
#include <Placement.hpp>

namespace HDL2Redstone {
class Component {
  public:
    Component(const Cell* CellPtr_);

    uint16_t getWidth() const { return CellPtr->getWidth(); }
    uint16_t getHeight() const { return CellPtr->getHeight(); }
    uint16_t getLength() const { return CellPtr->getLength(); }
    const std::string& getType() const { return CellPtr->getType(); }

    std::vector<std::string> getPinNames() const { return CellPtr->getPinNames(); }
    Direction getPinDir(const std::string& PinName_) const { return CellPtr->getPinDir(PinName_); }
    Facing getPinFacing(const std::string& PinName_) const;
    std::tuple<uint16_t, uint16_t, uint16_t> getPinLocation(const std::string& PinName_) const;
    bool getPlaced() const { return Placed; }
    void setPlaced(bool Placed_) { Placed = Placed_; }

    void setPlacement(uint16_t X_, uint16_t Y_, uint16_t Z_, Orientation Orient_) {
        P.X = X_;
        P.Y = Y_;
        P.Z = Z_;
        P.Orient = Orient_;
        Placed = true;
    }

    void clearPlacement() { Placed = false; }

    // Post placement method
    // TODO: Make sure they are only called when Placed is true
    const Placement& getPlacement() const { return P; }

    std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>> getRange() const;
    // Return schematic specified by CellStructName
    const Schematic& getSchematic() const { return CellPtr->getSchematic(); }

  private:
    const Cell* CellPtr;
    bool Placed;
    // Post placement data structure
    Placement P;
    friend std::ostream& operator<<(std::ostream& out, const Component& Component_);
};
} // namespace HDL2Redstone
