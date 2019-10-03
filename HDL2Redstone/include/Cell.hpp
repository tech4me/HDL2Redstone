#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <Schematic.hpp>

namespace HDL2Redstone {
class Cell {
    enum class Direction { Input, Output, Inout };
    enum class Orientation { North, South, West, East };
    enum class Facing { Up, Down, North, South, East, West };

  public:
    Cell() = delete;

    Cell(const std::string& Type_, const std::string& CellLibDir_,
         const std::map<std::string, std::map<std::string, std::string>>& Pin_, const std::string& SchematicName_);
    const std::string& getType() const { return Type; }
    const Schematic& getSchematic() const { return Schem; }

  private:
    std::string Type;

    class PinInfo {
      public:
        PinInfo(std::map<std::string, std::string> JsonInfo_);
        const Facing& getFacing() const { return Face; }
        const Direction& getDirection() const { return Dir; }

        Direction Dir;
        Facing Face;
    };
    // <pin_name, Other info:direction, function etc>
    std::map<std::string, PinInfo> Pins;
    Schematic Schem;

    friend std::ostream& operator<<(std::ostream& out, const Cell& Cell_);
};
} // namespace HDL2Redstone
