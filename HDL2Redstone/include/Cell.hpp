#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <Schematic.hpp>

namespace HDL2Redstone {

enum class Direction { Input, Output, Inout };
// enum class Orientation { North, South, West, East };
enum class Facing { Up, Down, North, South, East, West };

struct Pin {
    Direction Dir;
    Facing Face;
    std::tuple<uint16_t, uint16_t, uint16_t> Location;
};

class Cell {

  public:
    Cell() = delete;

    Cell(const std::string& Type_, const std::string& CellLibDir_, const std::map<std::string, Pin> Pins_,
         const std::string& SchematicName_);
    uint16_t getWidth() const { return Schem.getWidth(); }
    uint16_t getHeight() const { return Schem.getHeight(); }
    uint16_t getLength() const { return Schem.getLength(); }
    const std::string& getType() const { return Type; }
    const Schematic& getSchematic() const { return Schem; }
    std::vector<std::string> getPinNames() const;
    Direction getPinDir(const std::string& PinName_) const { return Pins.at(PinName_).Dir; }
    Facing getPinFacing(const std::string& PinName_) const { return Pins.at(PinName_).Face; }
    const std::tuple<uint16_t, uint16_t, uint16_t>& getPinLocation(std::string PinName_) const {
        return Pins.at(PinName_).Location;
    }

  private:
    std::string Type;

    // <pin_name, Other info:direction, function etc>
    std::map<std::string, Pin> Pins;
    Schematic Schem;

    friend std::ostream& operator<<(std::ostream& out, const Cell& Cell_);
};

} // namespace HDL2Redstone
