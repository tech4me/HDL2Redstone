#pragma once

#include <ModuleNetlist.hpp>
#include <string>
namespace HDL2Redstone {
class Placer {
  public:
    bool placing(ModuleNetlist& MN, uint16_t Width, uint16_t Height, uint16_t Length);
};
}