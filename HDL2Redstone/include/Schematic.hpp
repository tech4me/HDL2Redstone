#pragma once

#include <map>
#include <string>
#include <vector>

#include <io/stream_reader.h>

#include <Placement.hpp>

namespace HDL2Redstone {
class Schematic {
  public:
    // Create an empty Schematic
    Schematic(uint16_t Width_, uint16_t Height_, uint16_t Length_);
    // Load a Schematic from file
    Schematic(const std::string& File);

    uint16_t getWidth() const { return Width; }
    uint16_t getHeight() const { return Height; }
    uint16_t getLength() const { return Length; }

    void setWidth(uint16_t Width_) { Width = Width_; }
    void setHeight(uint16_t Height_) { Height = Height_; }
    void setLength(uint16_t Length_) { Length = Length_; }

    void insertSubSchematic(const Placement& P_, const Schematic& Schem_);
    void exportSchematic(const std::string& File_) const;

  private:
    uint16_t Width;
    uint16_t Height;
    uint16_t Length;
    std::vector<int32_t> Offset;
    int32_t PaletteMax;
    std::map<int32_t, std::string> InvertPalette;
    std::vector<int32_t> BlockData;
};
} // namespace HDL2Redstone
