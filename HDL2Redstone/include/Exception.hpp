#pragma once

#include <iostream>
#include <stdexcept>
#include <string>

#ifdef NDEBUG
#define DOUT(...)
#else
#define DOUT(...) (std::cout __VA_ARGS__)
#endif

namespace HDL2Redstone {
class Exception : public std::runtime_error {
  public:
    Exception() = delete;
    explicit Exception(const std::string& Msg_);
};

} // namespace HDL2Redstone
