#pragma once
#include <io/stream_reader.h>
#include <string>

namespace HDL2Redstone {
class Parameters {
  public:
    enum class Orientation { ZeroCW, OneCW, TwoCW, ThreeCW };
    Parameters();
    Parameters(std::string& CellStructure_, int16_t X_ = 0, int16_t Y_ = 0, int16_t Z_ = 0,
               Orientation Turn_ = Orientation::ZeroCW);
    void SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Parameters::Orientation Turn_);

  private:
    std::string CellStructure;
    int16_t X, Y, Z;
    Orientation Turn;
};
}