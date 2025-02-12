#pragma once

#include <string>

namespace HDL2Redstone {

struct Coordinate {
    uint16_t X;
    uint16_t Y;
    uint16_t Z;
};

enum class Orientation { ZeroCW, OneCW, TwoCW, ThreeCW };
class Placement {
  public:
    uint16_t X;
    uint16_t Y;
    uint16_t Z;
    Orientation Orient;
};
} // namespace HDL2Redstone
