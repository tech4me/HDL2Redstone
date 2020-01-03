#include <Connection.hpp>

using namespace HDL2Redstone;

Connection::Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_) : Name(Name_) {
    PortConnection.push_back(std::make_tuple(ComponentPtr_, PortName_, Connection::Parameters()));
}

const std::string& Connection::getName() const { return Name; }

const std::vector<std::tuple<Component*, std::string, Connection::Parameters>>& Connection::getPortConnection() const {
    return PortConnection;
}

void Connection::addSink(Component* ComponentPtr_, const std::string& PortName_) {
    PortConnection.push_back(std::make_tuple(ComponentPtr_, PortName_, Connection::Parameters()));
}

std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> Connection::checkRouteResult() {
    std::map<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t> ResultsMap;
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> IllegalPoints;
    for (const auto R : Result) {
        // std::cout<<"in check route result, " <<std::get<0>(R.coord)<<" "<<std::get<1>(R.coord)<<"
        // "<<std::get<2>(R.coord)<<std::endl;
        const auto it = ResultsMap.find(R.coord);
        if (it != ResultsMap.end()) {
            switch (it->second) {
            case 0:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord) - 1, std::get<2>(R.coord), 0});
                break;
            case 1:
            case 2:
            case 3:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord), std::get<2>(R.coord), 1});
                break;
            case 4:
                IllegalPoints.insert({std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord), 0});
                break;
            default:
                std::cout << "wrong" << std::endl;
                break;
            }
        } else {
            uint16_t x = std::get<0>(R.coord);
            uint16_t y = std::get<1>(R.coord);
            uint16_t z = std::get<2>(R.coord);
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y + 2, z}, 0));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y + 1, z}, 1));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y, z}, 2));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y - 1, z}, 3));
            ResultsMap.insert(std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, uint16_t>({x, y - 2, z}, 4));
        }
    }
    return IllegalPoints;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Connection& Connection_) {
    out << "Connection Name:" << Connection_.Name << std::endl;
    for (const auto& T : Connection_.PortConnection) {
        out << "    " << *(std::get<0>(T)) << "    Port Name:" << std::get<1>(T) << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
