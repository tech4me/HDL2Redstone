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

  public:
    Cell() = delete;

    Cell(const std::string& Type_, const std::string& CellLibDir_,
         const std::map<std::string, std::map<std::string, std::string>>& Pin_,
         const std::map<std::string, std::map<std::string, std::string>>& Schematics_);
    const std::string& getType() const { return Type; }
    const Schematic& getSchematic(const std::string& CellStructName) const {
        return Schematics.at(CellStructName).Schem;
    }

  private:
    std::string Type;

    class PinInfo {
      public:
        PinInfo(std::map<std::string, std::string> JsonInfo_);

        Direction Dir;
    };
    // <pin_name, Other info:direction, function etc>
    std::map<std::string, PinInfo> Pins;

    class SchemaInfo {
      public:
        SchemaInfo(const std::string& SchemaPath_, const std::map<std::string, std::string>& JsonInfo_);
        // will store timing, volume, constraint info
        // Orientation Ori;
        Schematic Schem;
    };
    // <schema_name, Other info:orientation, timing etc >
    std::map<std::string, SchemaInfo> Schematics;
};
} // namespace HDL2Redstone
