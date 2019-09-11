#pragma once

#include <iostream>
#include <string>

#include <Component.hpp>

namespace HDL2Redstone {
class Connection {
  public:
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_);
    const std::string& getName() const;
    const std::vector<std::pair<Component*, std::string>>& getPortNames() const;
    // add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);

  private:
    const std::string Name;
    std::vector<std::pair<Component*, std::string>> PortName;
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
