#pragma once

#include <string>

#include <Component.hpp>

namespace HDL2Redstone {
class Connection {
  public:
    Connection(const std::string& Name_, Component* Component_, const std::string& PortName_);
};

// bool addSource();
// Check connection's exsisting sinks, add port to sink
// bool addSink(Port*);
} // namespace HDL2Redstone
