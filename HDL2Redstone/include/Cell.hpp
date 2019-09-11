#pragma once

#include <map>
#include <string>

#include <Schematic.hpp>

namespace HDL2Redstone {
class Cell {
    enum class Direction { Input, Output, Inout };

  public:
    Cell() = delete;

    Cell(const std::string& Type_, const std::map<std::string, std::map<std::string, std::string>>& Pin_);
    const std::string& getType() const;

  private:
    Schematic Sch;
    std::string Type;

    class PinInfo {
      public:
        PinInfo(std::map<std::string, std::string> JsonInfo_);

        Direction Dir;
    };
    // <pin_name, direction_info>
    std::map<std::string, PinInfo> Pins;
    // Here we should also store timing, volume, and constraint info
};
} // namespace HDL2Redstone
