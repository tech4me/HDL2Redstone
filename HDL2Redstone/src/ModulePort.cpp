#include <ModulePort.hpp>

using namespace HDL2Redstone;

ModulePort::ModulePort(const std::string& Name_, PortType PT_) : Name(Name_), PT(PT_) {}

const std::string& ModulePort::getName() const { return Name; }
