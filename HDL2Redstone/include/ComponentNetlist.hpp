#pragma once

#include <string>
#include <vector>

#include <Component.hpp>
#include <Connection.hpp>

namespace HDL2Redstone {
class ComponentNetlist {
  public:
    ComponentNetlist(const std::string& File);

  private:
    std::vector<Component> Components;
    std::vector<Connection> Connections;
};
} // namespace HDL2Redstone
