#pragma once

#include <string>

#include <Component.hpp>

namespace HDL2Redstone {
class Connection {
  public:
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_);

    const std::string& getName() const;

    // Check connection's exsisting sinks, add component port to sink
    bool addSink(Component* ComponentPtr_, const std::string& PortName_);

  private:
    std::string Name;
};
} // namespace HDL2Redstone
