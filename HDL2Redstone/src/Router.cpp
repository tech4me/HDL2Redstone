#include <cmath>
#include <queue>
#include <set>

#include <Component.hpp>
#include <Connection.hpp>
#include <Router.hpp>

using namespace HDL2Redstone;
std::tuple<uint16_t, uint16_t, uint16_t> Router::updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc,
                                                                           Facing Fac) {
    std::cout << "----" << std::get<0>(Loc) << " " << std::get<1>(Loc) << " " << std::get<2>(Loc) << " " << std::endl;
    if (Fac == Facing::North) {
        std::cout << 1 << std::endl;
        UsedSpace[std::get<0>(Loc)][std::get<1>(Loc)][std::get<2>(Loc) - 1] = 0;
        return std::make_tuple(std::get<0>(Loc), std::get<1>(Loc), std::get<2>(Loc) - 1);
    }
    if (Fac == Facing::East) {
        std::cout << 2 << std::endl;
        UsedSpace[std::get<0>(Loc) + 1][std::get<1>(Loc)][std::get<2>(Loc)] = 0;
        return std::make_tuple(std::get<0>(Loc) + 1, std::get<1>(Loc), std::get<2>(Loc));
    }
    if (Fac == Facing::South) {
        std::cout << 3 << std::endl;
        UsedSpace[std::get<0>(Loc)][std::get<1>(Loc)][std::get<2>(Loc) + 1] = 0;
        return std::make_tuple(std::get<0>(Loc), std::get<1>(Loc), std::get<2>(Loc) + 1);
    }
    if (Fac == Facing::West) {
        std::cout << 4 << std::endl;
        UsedSpace[std::get<0>(Loc) - 1][std::get<1>(Loc)][std::get<2>(Loc)] = 0;
        return std::make_tuple(std::get<0>(Loc) - 1, std::get<1>(Loc), std::get<2>(Loc));
    }
    return Loc;
}
Router::Router(const Design& D) {
    std::tuple<uint16_t, uint16_t, uint16_t> Space = D.getSpace();
    UsedSpace = new bool**[std::get<0>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        UsedSpace[i] = new bool*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            UsedSpace[i][j] = new bool[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                UsedSpace[i][j][k] = 0;
            }
        }
    }
    auto& Components_ = D.getModuleNetlist().getComponents();
    auto& Connections_ = D.getModuleNetlist().getConnections();
    std::vector<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>>
        UsedComponentSpace;
    std::vector<std::vector<std::tuple<int16_t, int16_t, int16_t>>> UsedConnectionSpace;
    for (auto const& i : Components_) {
        UsedComponentSpace.push_back(i->getRange());
    } // TO DO: move in design.route
      /*
          for (auto const& j : Connections_) {
              const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
                  j->getPortConnection();
              for (auto const& k : PortConnection_) {
                  if (!(std::get<2>(k).getParameters().empty())) {
                      UsedConnectionSpace.push_back(std::get<2>(k).getParameters());
                  }
              }
          } // TD DO: move in design.route
      */

    for (auto const& i : UsedComponentSpace) {
        for (uint16_t j = std::get<0>(i.first); j < std::get<0>(i.second); j++) {
            for (uint16_t k = std::get<1>(i.first); k < std::get<1>(i.second); k++) {
                for (uint16_t z = std::get<2>(i.first); z < std::get<2>(i.second); z++) {
                    UsedSpace[j][k][z] = 1;
                    if (j > 0)
                        UsedSpace[j - 1][k][z] = 1;
                    if (j < std::get<0>(Space) - 1)
                        UsedSpace[j + 1][k][z] = 1;
                    /*if (k > 0)
                        UsedSpace[j][k - 1][z] = 1;
                    if (k < std::get<1>(Space) - 1)
                        UsedSpace[j][k + 1][z] = 1; // TODO veritcal range should be reconsidered
                        */
                    if (z > 0)
                        UsedSpace[j][k][z - 1] = 1;
                    if (z < std::get<2>(Space) - 1)
                        UsedSpace[j][k][z + 1] = 1;
                }
            }
        }
    }
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                if (j == 1)
                    std::cout << UsedSpace[i][j][k] << " ";
            }
        }
        std::cout << std::endl;
    }
}
void Router::route(Design& D) {
    // auto& Components_ = D.getModuleNetlist().getComponents();
    auto& Connections_ = D.getModuleNetlist().getConnections();
    for (auto& it : Connections_) {
        std::cout << it->getName() << std::endl;
        std::cout << *it << std::endl;
        flatRoute(D, *it);
        // std::cout<<"DDDD"<<std::endl;
        // updateUsedSpace(*it, D.getSpace());
        // std::cout<<"HHHH"<<std::endl;
    }
}
bool Router::flatRoute(Design& D, Connection& C) {
    // implement dijkstra
    class Point {
      public:
        std::tuple<uint16_t, uint16_t, uint16_t> Loc;
        int cost;
        Point* P;
        bool visited;
    };
    class PointCompare {
      public:
        bool operator()(const Point& lhs, const Point& rhs) const { return lhs.cost > rhs.cost; }
    };
    std::tuple<uint16_t, uint16_t, uint16_t> Space = D.getSpace();
    Point P_[std::get<0>(Space)][std::get<1>(Space)][std::get<2>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                P_[i][j][k].P = NULL;
                P_[i][j][k].visited = 0;
                P_[i][j][k].Loc = std::make_tuple(i, j, k);
            }
        }
    }
    std::tuple<uint16_t, uint16_t, uint16_t> start;
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> end;
    std::tuple<uint16_t, uint16_t, uint16_t> startPin;
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> endPin;

    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
        C.getPortConnection();

    // source
    auto SrcFacing = (std::get<0>(PortConnection_[0]))->getPinFacing(std::get<1>(PortConnection_[0]));
    startPin = (std::get<0>(PortConnection_[0]))->getPinLocation(std::get<1>(PortConnection_[0]));
    start = Router::updateSinglePortUsedSpace(startPin, SrcFacing);
    for (auto it = PortConnection_.begin() + 1; it != PortConnection_.end(); ++it) {
        auto temp = std::get<0>(*it)->getPinLocation(std::get<1>(*it));
        endPin.push_back(temp);
        // temp = std::make_tuple(std::get<0>(temp),std::get<1>(temp),std::get<2>(temp) );
        auto endFace = (std::get<0>(*it))->getPinFacing(std::get<1>(*it));
        end.push_back(Router::updateSinglePortUsedSpace(temp, endFace));
    }
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> endTemp = end;
    std::priority_queue<Point, std::vector<Point>, PointCompare> Q;
    std::cout << std::get<0>(start) << " " << std::get<1>(start) << " " << std::get<2>(start) << " " << std::endl;
    P_[std::get<0>(start)][std::get<1>(start)][std::get<2>(start)].cost = 0;

    Q.push(P_[std::get<0>(start)][std::get<1>(start)][std::get<2>(start)]);
    std::cout << "GGGG" << std::endl;
    bool done = 0;

    while (!Q.empty()) {
        Point TempP = Q.top();
        Q.pop();
        TempP.visited = 1;
        for (auto it = endTemp.begin(); it != endTemp.end(); ++it) {
            if (TempP.Loc == *it) {
                // std::cout <<"Location: "<<std::get<0>(TempP.Loc)<<" "<<std::get<1>(TempP.Loc)<<"
                // "<<std::get<2>(TempP.Loc)<<std::endl;
                endTemp.erase(it);
                break;
            }
        }
        if (endTemp.empty())
            break;

        if ((std::get<0>(TempP.Loc)) &&
            (!UsedSpace[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)])) {
            if ((P_[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost >
                 P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1)) {
                P_[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].P =
                    &P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)];
                P_[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost =
                    P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1;
            }

            if (!P_[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].visited)
                Q.push(P_[std::get<0>(TempP.Loc) - 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)]);
        }
        if ((std::get<0>(TempP.Loc) < std::get<0>(Space) - 1) &&
            (!UsedSpace[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)])) {
            if ((P_[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost >
                 P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1)) {
                P_[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].P =
                    &P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)];
                P_[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost =
                    P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1;
            }

            if (!P_[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].visited)
                Q.push(P_[std::get<0>(TempP.Loc) + 1][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)]);
        }
        if ((std::get<2>(TempP.Loc)) &&
            (!UsedSpace[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1])) {
            if ((P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1].cost >
                 P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1)) {
                P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1].P =
                    &P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)];
                P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1].cost =
                    P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1;
            }

            if (!P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1].visited)
                Q.push(P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) - 1]);
        }
        if ((std::get<2>(TempP.Loc) < std::get<2>(Space) - 1) &&
            (!UsedSpace[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1])) {
            if ((P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1].cost >
                 P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1)) {
                P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1].P =
                    &P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)];
                P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1].cost =
                    P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)].cost + 1;
            }

            if (!P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1].visited)
                Q.push(P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc) + 1]);
        }
        P_[std::get<0>(TempP.Loc)][std::get<1>(TempP.Loc)][std::get<2>(TempP.Loc)] = TempP;
    }
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>> Result;
    if (endTemp.empty()) {
        Point* ptr = NULL;
        for (auto it : end) {
            std::cout << "Location: " << std::get<0>(it) << " " << std::get<1>(it) << " " << std::get<2>(it)
                      << std::endl;
            Result.insert(std::make_tuple(std::get<0>(it), std::get<1>(it) - 1, std::get<2>(it)));
            ptr = P_[std::get<0>(it)][std::get<1>(it)][std::get<2>(it)].P;
            while (ptr != NULL) {
                std::cout << "Location: " << std::get<0>(ptr->Loc) << " " << std::get<1>(ptr->Loc) << " "
                          << std::get<2>(ptr->Loc) << std::endl;
                Result.insert(std::make_tuple(std::get<0>(ptr->Loc), std::get<1>(ptr->Loc) - 1, std::get<2>(ptr->Loc)));
                ptr = ptr->P;
            }
        }
    }
    for (const auto& entry : Result) {
        UsedSpace[std::get<0>(entry)][std::get<1>(entry)+1][std::get<2>(entry)] = 1;
        if (std::get<0>(entry) > 0)
            UsedSpace[std::get<0>(entry) - 1][std::get<1>(entry)+1][std::get<2>(entry)] = 1;
        if (std::get<0>(entry) < std::get<0>(Space) - 1)
            UsedSpace[std::get<0>(entry) + 1][std::get<1>(entry)+1][std::get<2>(entry)] = 1;
        if (std::get<2>(entry) > 0)
            UsedSpace[std::get<0>(entry)][std::get<1>(entry)+1][std::get<2>(entry) - 1] = 1;
        if (std::get<2>(entry) < std::get<2>(Space) - 1)
            UsedSpace[std::get<0>(entry)][std::get<1>(entry)+1][std::get<2>(entry) + 1] = 1;
    }
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                if (j == 1)
                    std::cout << UsedSpace[i][j][k] << " ";
            }
        }
        std::cout << std::endl;
    }
    C.Result = Result;
    return true;
}
bool Router::updateUsedSpace(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t> bound) {
    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
        C.getPortConnection();
    if (PortConnection_.empty()) {
        return false;
    }

    for (auto const& k : PortConnection_) {
        if (!(std::get<2>(k).getParameters().empty())) {
            const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> tempConnection = std::get<2>(k).getParameters();
            for (auto const& i : tempConnection) {
                UsedSpace[std::get<0>(i)][std::get<1>(i)][std::get<2>(i)] = 1;
                if (std::get<0>(i) > 0)
                    UsedSpace[std::get<0>(i) - 1][std::get<1>(i)][std::get<2>(i)] = 1;
                if (std::get<0>(i) < std::get<0>(bound) - 1)
                    UsedSpace[std::get<0>(i) + 1][std::get<1>(i)][std::get<2>(i)] = 1;
                /*if (std::get<1>(i) > 0)
                    UsedSpace[std::get<0>(i)][std::get<1>(i) - 1][std::get<2>(i)] = 1;
                if (std::get<1>(i) < std::get<1>(bound) - 1)
                    UsedSpace[std::get<0>(i)][std::get<1>(i) + 1][std::get<2>(i)] =
                        1; // TODO veritcal range should be reconsidered
                */
                if (std::get<2>(i) > 0)
                    UsedSpace[std::get<0>(i)][std::get<1>(i)][std::get<2>(i) - 1] = 1;
                if (std::get<2>(i) < std::get<2>(bound) - 1)
                    UsedSpace[std::get<0>(i)][std::get<1>(i)][std::get<2>(i) + 1] = 1;
            }
        }
    }
    return true;
}

bool Router::checkSingleRoute(const Design& D,
                              const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points) {
    auto& Components_ = D.getModuleNetlist().getComponents();
    auto& Connections_ = D.getModuleNetlist().getConnections();
    std::vector<std::pair<std::tuple<uint16_t, uint16_t, uint16_t>, std::tuple<uint16_t, uint16_t, uint16_t>>>
        UsedComponentSpace;
    std::vector<std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>> UsedConnectionSpace;
    for (auto const& i : Components_) {
        UsedComponentSpace.push_back(i->getRange());
    } // TO DO: move in design.route
    for (auto const& j : Connections_) {
        const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
            j->getPortConnection();
        for (auto const& k : PortConnection_) {
            if (!(std::get<2>(k).getParameters().empty())) {
                UsedConnectionSpace.push_back(std::get<2>(k).getParameters());
            }
        }
    } // TD DO: move in design.route
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

std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>
Router::flatRouteDirectLine(std::tuple<uint16_t, uint16_t, uint16_t> start,
                            std::tuple<uint16_t, uint16_t, uint16_t> end) {
    // Note: Here implement with tuple<x,z,y>
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> result;
    // result.push_back(start);
    uint16_t x_distance = abs(std::get<0>(start) - std::get<0>(end)) + 1;
    uint16_t z_distance = abs(std::get<1>(start) - std::get<1>(end)) + 1;
    uint16_t x_dir;
    uint16_t z_dir;
    if (std::get<0>(start) - std::get<0>(end) > 0) {
        x_dir = -1;
    } else if (std::get<0>(start) - std::get<0>(end) < 0) {
        x_dir = 1;
    } else {
        x_dir = 0;
    }

    if (std::get<1>(start) - std::get<1>(end) > 0) {
        z_dir = -1;
    } else if (std::get<1>(start) - std::get<1>(end) < 0) {
        z_dir = 1;
    } else {
        z_dir = 0;
    }

    if (x_dir == 0) {
        for (uint16_t i = 0; i < z_distance; i++) {
            result.push_back(std::make_tuple(std::get<0>(start), std::get<1>(start) + i, std::get<2>(start)));
        }
        return result;
    }
    if (z_dir == 0) {
        for (uint16_t i = 0; i < x_distance; i++) {
            result.push_back(std::make_tuple(std::get<0>(start) + i, std::get<1>(start), std::get<2>(start)));
        }
        return result;
    }

    uint16_t step;
    uint16_t remainder;
    uint16_t x_repair = 1;
    uint16_t z_repair = 0;
    if (x_distance >= z_distance) {
        remainder = x_distance % z_distance;
        step = x_distance / z_distance;

        if (remainder > 0) {
            remainder = x_distance % (z_distance - 1);
            step = x_distance / (z_distance - 1);
        } // TODO: need to re-update

        for (uint16_t i = 0; i < z_distance - 1; i++) {
            for (uint16_t j = 0; j < step; j++) {
                result.push_back(std::make_tuple(std::get<0>(start) + x_dir * (j + step * i),
                                                 std::get<1>(start) + z_dir * i, std::get<2>(start)));
            }
            result.push_back(std::make_tuple(std::get<0>(start) + x_dir * ((i + 1) * step - 1 + x_repair),
                                             std::get<1>(start) + z_dir * (i + z_repair), std::get<2>(start)));
            uint16_t temp_repair = x_repair;
            x_repair = z_repair;
            z_repair = temp_repair;
        }
        for (uint16_t k = 0; k < remainder; k++) {
            result.push_back(std::make_tuple(std::get<0>(start) + x_dir * (k + (z_distance - 1) * step),
                                             std::get<1>(end), std::get<2>(start)));
        }
    } else {
        remainder = z_distance % x_distance;
        step = z_distance / x_distance;
        if (remainder > 0) {
            remainder = z_distance % (x_distance - 1);
            step = z_distance / (x_distance - 1);
        } // TODO: need to re-update
        for (uint16_t i = 0; i < x_distance - 1; i++) {
            for (uint16_t j = 0; j < step; j++) {
                result.push_back(std::make_tuple(std::get<0>(start) + x_dir * i,
                                                 std::get<1>(start) + z_dir * (j + step * i), std::get<2>(start)));
            }
            result.push_back(std::make_tuple(std::get<0>(start) + x_dir * (i + x_repair),
                                             std::get<1>(start) + z_dir * ((i + 1) * step - 1 + z_repair),
                                             std::get<2>(start)));
            uint16_t temp_repair = x_repair;
            x_repair = z_repair;
            z_repair = temp_repair;
        }
        for (uint16_t k = 0; k < remainder; k++) {
            result.push_back(std::make_tuple(
                std::get<0>(end), std::get<1>(start) + z_dir * (k + (x_distance - 1) * step), std::get<2>(start)));
        }
    }
    return result;
}