#include <Connection.hpp>

using namespace HDL2Redstone;

Connection::Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_) : Name(Name_) {
    Connection::Parameters blank;
    PortConnection.push_back(std::make_tuple(ComponentPtr_, PortName_, blank));
}

const std::string& Connection::getName() const { return Name; }

const std::vector<std::tuple<Component*, std::string, Connection::Parameters>>& Connection::getPortConnection() const { 
    return PortConnection; 
}

void Connection::addSink(Component* ComponentPtr_, const std::string& PortName_) {
    Connection::Parameters blank;
    PortConnection.push_back(std::make_tuple(ComponentPtr_, PortName_, blank));
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Connection& Connection_) {
    out << "Connection Name:" << Connection_.Name << std::endl;
    for (const auto& T : Connection_.PortConnection) {
        out << "    " << * (std::get<0>(T)) << "    Port Name:" << std::get<1>(T) << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone