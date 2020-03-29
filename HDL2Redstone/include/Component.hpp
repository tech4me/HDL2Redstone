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
    int getDelay() const { return CellPtr->getDelay(); }
    std::vector<std::string> getPinNames() const { return CellPtr->getPinNames(); }
    Direction getPinDir(const std::string& PinName_) const { return CellPtr->getPinDir(PinName_); }
    Facing getPinFacing(const std::string& PinName_) const;
    std::tuple<uint16_t, uint16_t, uint16_t> getPinLocation(const std::string& PinName_) const;
    Coordinate getPinLocationWithPlacement(const std::string& PinName_, const Placement& P_) const;
    bool getPlaced() const { return Placed; }
    void setPlaced(bool Placed_) { Placed = Placed_; }
    void setForcePlaced() {
        Placed = true;
        ForcePlaced = true;
    }
    const std::string& getName() const { return Name; }
    void setName(const std::string& Name_) { Name = Name_; }

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

    std::pair<Coordinate, Coordinate> getRange() const { return getRangeWithPlacement(P); }
    std::pair<Coordinate, Coordinate> getRangeWithPlacement(const Placement& Placement_) const;
    // Return schematic specified by CellStructName
    const Schematic& getSchematic() const { return CellPtr->getSchematic(); }

  private:
    const Cell* CellPtr;
    bool Placed;
    bool ForcePlaced;
    std::string Name; // for i/o set by user
    // Post placement data structure
    Placement P;
    friend std::ostream& operator<<(std::ostream& out, const Component& Component_);
};
} // namespace HDL2Redstone
