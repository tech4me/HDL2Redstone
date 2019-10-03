#include <Component.hpp>
#include <Connection.hpp>
#include <Router.hpp>

using namespace HDL2Redstone;

bool Router::flatRoute(Design& D, Connection& C) { return true; }

bool Router::checkSingleRoute(const Design& D,
                              const std::vector<std::tuple<int16_t, int16_t, int16_t>> connection_points) {
    auto& Components_ = D.getModuleNetlist().getComponents();
    auto& Connections_ = D.getModuleNetlist().getConnections();
    std::vector<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>>
        UsedComponentSpace;
    std::vector<std::vector<std::tuple<int16_t, int16_t, int16_t>>> UsedConnectionSpace;
    for (auto const& i : Components_) {
        UsedComponentSpace.push_back(i->getRange());
    }
    for (auto const& j : Connections_) {
        const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
            j->getPortConnection();
        for (auto const& k : PortConnection_) {
            if (!(std::get<2>(k).getParameters().empty())) {
                UsedConnectionSpace.push_back(std::get<2>(k).getParameters());
            }
        }
    }
    for (auto const& l0 : connection_points) {
        for (auto const& l1 : UsedComponentSpace) {
            if ((std::get<0>(l0) > std::get<0>(l1.second) + 1) || (std::get<0>(l0) < std::get<0>(l1.first) - 1) ||
                (std::get<1>(l0) > std::get<1>(l1.second) + 1) || (std::get<1>(l0) < std::get<1>(l1.first) - 1) ||
                (std::get<2>(l0) > std::get<2>(l1.second) + 1) || (std::get<2>(l0) < std::get<2>(l1.first) - 1))
                return false;
        }
        for (auto const& l2 : UsedConnectionSpace) {
            for (auto const& l3 : l2) {
                if ((l0 == std::make_tuple(std::get<0>(l3) - 1, std::get<1>(l3), std::get<2>(l3))) ||
                    (l0 == std::make_tuple(std::get<0>(l3) + 1, std::get<1>(l3), std::get<2>(l3))) ||
                    (l0 == std::make_tuple(std::get<0>(l3), std::get<1>(l3) + 1, std::get<2>(l3))) ||
                    (l0 == std::make_tuple(std::get<0>(l3), std::get<1>(l3) - 1, std::get<2>(l3))) ||
                    (l0 == std::make_tuple(std::get<0>(l3), std::get<1>(l3), std::get<2>(l3) + 1)) ||
                    (l0 == std::make_tuple(std::get<0>(l3), std::get<1>(l3), std::get<2>(l3) - 1)) ||
                    (l0 == std::make_tuple(std::get<0>(l3), std::get<1>(l3), std::get<2>(l3))))
                    return false;
            }
        }
    }
    return true;
}
