#pragma once

#include <stdexcept>
#include <string>

namespace HDL2Redstone {
class Exception : public std::runtime_error {
  public:
    Exception() = delete;
    explicit Exception(const std::string& Msg_);
};

} // namespace HDL2Redstone
