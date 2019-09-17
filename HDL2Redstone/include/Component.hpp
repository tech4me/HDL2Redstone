#pragma once

#include <iostream>
#include <string>

#include <Cell.hpp>

namespace HDL2Redstone {
class Component {
  public:
    enum class Orientation { ZeroCW, OneCW, TwoCW, ThreeCW };
    Component(const Cell* CellPtr_);
    void SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Orientation Turn_);
    friend std::ostream& operator<<(std::ostream& out, const Component& Component_);

  private:
    class Parameters {
      public:
        Parameters(std::string CellStructure_ = "NOT", int16_t X_ = 0, int16_t Y_ = 0, int16_t Z_ = 0,
                   Orientation Turn_ = Orientation::ZeroCW);
        void SetParameters(std::string& CellStructure_, int16_t X_, int16_t Y_, int16_t Z_, Orientation Turn_);

        std::string CellStructure;
        int16_t X, Y, Z;
        Orientation Turn;
    };
    const Cell* CellPtr;
    Parameters Param;
};
} // namespace HDL2Redstone
