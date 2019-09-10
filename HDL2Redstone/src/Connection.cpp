#include <Connection.hpp>

using namespace HDL2Redstone;

Connection::Connection(const std::string& Name_, Component* Component_, const std::string& PortName_) : Name(Name_) {}

const std::string& Connection::getName() const { return Name; }
