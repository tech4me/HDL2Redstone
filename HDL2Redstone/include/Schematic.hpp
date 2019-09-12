#pragma once

#include <string>
#include <vector>

#include <io/stream_reader.h>

namespace HDL2Redstone {
class Schematic {
  public:
    Schematic(const std::string& File);

    int16_t getWidth() const { return Width; }
    int16_t getHeight() const { return Height; }
    int16_t getLength() const { return Length; }
    const std::string& getMaterials() const { return Materials; }
    const std::vector<int8_t>& getBlocks() const { return Blocks; }
    const std::vector<int8_t>& getData() const { return Data; }

    void setWidth(int16_t Width_) { Width = Width_; }
    void setHeight(int16_t Height_) { Height = Height_; }
    void setLength(int16_t Length_) { Length = Length_; }
    void setMaterials(const std::string& Materials_) { Materials = Materials_; }
    void setBlocks(const std::vector<int8_t>& Blocks_) { Blocks = Blocks_; }
    void setData(const std::vector<int8_t>& Data_) { Data = Data_; }

  private:
    int16_t Width;
    int16_t Height;
    int16_t Length;
    std::string Materials;
    std::vector<int8_t> Blocks;
    std::vector<int8_t> Data;
};
} // namespace HDL2Redstone
