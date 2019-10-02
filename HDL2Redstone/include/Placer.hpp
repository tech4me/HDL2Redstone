#pragma once

#include <string>

#include <Design.hpp>

namespace HDL2Redstone {
class Placer {
  public:
    Placer(Design& D_);

    bool place();
    bool checkComponentLegality() const;

  private:
    Design& D;
};
} // namespace HDL2Redstone
