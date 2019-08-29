#pragma once

#include <io/stream_reader.h>
#include <string>
#include <vector>

class Schematic {
  public:
    bool loadSchematic(const std::string& File);

    int16_t getWidth() const { return Width; }
    int16_t getHeight() const { return Height; }
    int16_t getLength() const { return Length; }
    const std::string& getMaterials() const { return Materials; }
    const std::vector<int8_t>& getBlocks() const { return Blocks; }
    const std::vector<int8_t>& getData() const { return Data; }

  private:
    int16_t Width;
    int16_t Height;
    int16_t Length;
    std::string Materials;
    std::vector<int8_t> Blocks;
    std::vector<int8_t> Data;
};
