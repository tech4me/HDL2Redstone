#include <Connection.hpp>

using namespace HDL2Redstone;

Connection::Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_) : Name(Name_) {
    PortName.push_back(std::make_pair(ComponentPtr_, PortName_));
}

const std::string& Connection::getName() const { return Name; }

const std::vector<std::pair<Component*, std::string>>& Connection::getPortNames() const { return PortName; }

void Connection::addSink(Component* ComponentPtr_, const std::string& PortName_) {
    PortName.push_back(std::make_pair(ComponentPtr_, PortName_));
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Connection& Connection_) {
    out << "Connection Name:" << Connection_.Name << std::endl;
    for (const auto& T : Connection_.PortName) {
        out << *(T.first) << "Port Name:" << T.second << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone