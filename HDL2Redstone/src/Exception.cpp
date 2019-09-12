#include <Exception.hpp>

using namespace HDL2Redstone;

Exception::Exception(const std::string& Msg_) : std::runtime_error(Msg_) {}
