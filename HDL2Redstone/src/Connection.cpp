#include <Connection.hpp>

using namespace HDL2Redstone;

Connection::Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_)
    : Name(Name_) {}

const std::string& Connection::getName() const { return Name; }

bool Connection::addSink(Component* ComponentPtr_, const std::string& PortName_) { return false; }
