#pragma once

#include <string>

namespace HDL2Redstone {
enum class PortType { Input, Output };
class ModulePort {
  public:
    ModulePort(const std::string& Name_, PortType PT_);

    const std::string& getName() const;

  private:
    std::string Name;
    PortType PT;
};
} // namespace HDL2Redstone
