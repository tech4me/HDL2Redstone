#include <algorithm>
#include <cmath>
#include <queue>
#include <set>

#include <Component.hpp>
#include <Connection.hpp>
#include <Exception.hpp>
#include <Router.hpp>

using namespace HDL2Redstone;

Router::Router(Design& D_)
    : D(D_), UsedSpace(D.Width * D.Height * D.Length, 0), WI(D.Width * D.Height * D.Length),
      Points(D.Width * D.Height * D.Length), FailedWireSingleRouting(nullptr) {
    for (uint16_t X = 0; X != D.Width; ++X) {
        for (uint16_t Y = 0; Y != D.Height; ++Y) {
            for (uint16_t Z = 0; Z != D.Length; ++Z) {
                getPoint(X, Y, Z).Loc.x = X;
                getPoint(X, Y, Z).Loc.y = Y;
                getPoint(X, Y, Z).Loc.z = Z;
                getPoint(X, Y, Z).base_cost = 0;
                getPoint(X, Y, Z).mul_cost = 1;
                getPoint(X, Y, Z).cost = D.Width * D.Height * D.Length;
                getPoint(X, Y, Z).length = -1;
                getPoint(X, Y, Z).ori = HDL2Redstone::Orientation::ZeroCW;
                getPoint(X, Y, Z).P = nullptr;
                getPoint(X, Y, Z).visited = 0;
                getPoint(X, Y, Z).inserted = 0;
            }
        }
    }
    initStructs();
}

bool Router::route() {
    // std::cout<<"\nINIT FOUND wire status: "<<UsedSpace[21][0][10]<<std::endl;
    //  if(WI[33][12][9].CPtrs)
    //  std::cout<<"\nINIT wire name: "<<WI[21][0][10].CPtrs->getName()<<" "<<UsedSpace[33][12][9]<<std::endl;
    /*
    check unablerouting is 0 or 1 or 2 or 3
    to reroute everything,
    1. reconstruct router
    2. keep all wires results, setRouted(0)
    3. after routing this wire, check every wire can keep same result or reroute
    */

    // for (auto& it : Connections_) {
    //     if(it->getName()=="$abc$80$new_n29_"){
    //         auto SourcePortConnection_ = it->getSourcePortConnection();
    //         auto PortConnection_ = it->getSinkPortConnections();
    //         auto startPin = ((SourcePortConnection_).first)->getPinLocation((SourcePortConnection_).second);
    //         std::cout << "debugging " << std::get<0>(startPin) << ", " << std::get<1>(startPin) << ", " <<
    //         std::get<2>(startPin)<< std::endl;
    //     }
    // }
    // for (auto& it : Connections_) {
    //     auto SourcePortConnection_ = it->getSourcePortConnection();
    //     auto PortConnection_ = it->getSinkPortConnections();
    //     auto startPin = ((SourcePortConnection_).first)->getPinLocation((SourcePortConnection_).second);
    //     if(std::get<0>(startPin)==23 && std::get<1>(startPin)==11 && std::get<2>(startPin)==25){
    //         std::cout << "startpin debugging " << it->getName()<<" "<<std::get<0>(startPin) << ", " <<
    //         std::get<1>(startPin) << ", " << std::get<2>(startPin)<< std::endl;
    //     }
    //     for (auto i = PortConnection_.begin(); i != PortConnection_.end(); ++i) {
    //         auto temp = i->first->getPinLocation(i->second);
    //         if(std::get<0>(temp)==23 && std::get<1>(temp)==11 && std::get<2>(temp)==25){
    //             std::cout << "endpin debugging " << it->getName() <<" "<<std::get<0>(temp) << ", " <<
    //             std::get<1>(temp) << ", " << std::get<2>(temp)<< std::endl;
    //         }
    //     }
    // }
    auto& Connections = D.getModuleNetlist().getConnections();
    for (int i = 0; i < Connections.size(); i++) {
        bool all_routed = false;
        FailedWireSingleRouting = nullptr;
        for (auto& it : Connections) {
            if (it->Result.empty()) {
                std::for_each(Points.begin(), Points.end(), [this](Point& P) {
                    P.cost = D.Width * D.Height * D.Length;
                    P.length = -1;
                    P.ori = HDL2Redstone::Orientation::ZeroCW;
                    P.P = nullptr;
                    P.visited = 0;
                    P.inserted = 0;
                });
                std::cout << "Routing: " << it->getName() << std::endl;
                // DOUT( << "Routing: " << it->getName() << std::endl);
                if (!regularRoute(*it)) {
                    DOUT(<< "Routing: " << it->getName() << " failed" << std::endl);
                }
                //  std::cout<<"\nFOUND wire status: "<<UsedSpace[9][0][24]<<"
                //  "<<WI[9][0][24].ComponentSpace<<std::endl; if(!WI[9][0][24].CPtrs.empty()){
                //      for (auto it: WI[9][0][24].CPtrs)
                //     std::cout<<"\nFOUND wire name: "<<it->getName()<<std::endl;
                //  }else{
                //      std::cout<<"\nFOUND wire EMPTY"<<std::endl;
                //  }
                if (FailedWireSingleRouting) {
                    break;
                }
            }
        }
        if (!FailedWireSingleRouting) {
            DOUT(<< "FUll Routing Success!!!" << std::endl);
            break;
        } else if (FailedWireSingleRouting->getUnableRouting() /* == 1*/) {
            // Clear and reinit UsedSpace, WI, Points
            std::fill(UsedSpace.begin(), UsedSpace.end(), 0);
            std::for_each(WI.begin(), WI.end(), [](WireInfo& W) {
                W.ComponentSpace = 0;
                W.CPtrs.clear();
            });
            std::for_each(Points.begin(), Points.end(), [this](Point& P) {
                P.cost = D.Width * D.Height * D.Length;
                P.length = -1;
                P.ori = HDL2Redstone::Orientation::ZeroCW;
                P.P = nullptr;
                P.visited = 0;
                P.inserted = 0;
            });
            initStructs();

            FailedWireSingleRouting->setUnableRouting(2);
            FailedWireSingleRouting->Result.clear();
            FailedWireSingleRouting->RouteResult.clear();
            DOUT(<< "Try Routing: " << FailedWireSingleRouting->getName() << " first" << std::endl);
            auto FailedWireSingleRoutingTemp = FailedWireSingleRouting;
            FailedWireSingleRouting = nullptr;
            if (!regularRoute(*FailedWireSingleRoutingTemp)) {
                DOUT(<< "Routing: " << FailedWireSingleRoutingTemp->getName() << " failed" << std::endl);
            } else {
                for (auto& it : Connections) {
                    if (FailedWireSingleRoutingTemp->getName() != it->getName()) {
                        checkKeepOrUpdate(*it);
                    }
                }
            }
        }
        if (i == Connections.size() - 1) {
            DOUT(<< "Routing Times are not enough" << std::endl);
        }
    }
    return false;
}

void Router::initStructs() {
    const auto& Components = D.MN.getComponents();
    const auto& Connections = D.MN.getConnections();

    for (const auto& C : Components) {
        const auto& [Begin, End] = C->getRange();
        for (auto X = Begin.X; X != End.X; ++X) {
            for (auto Y = Begin.Y; Y != End.Y; ++Y) {
                for (auto Z = Begin.Z; Z != End.Z; ++Z) {
                    getUsedSpace(X, Y, Z) = 1;
                    getWireInfo(X, Y, Z).ComponentSpace = 1;
                    if (X > 0) {
                        getUsedSpace(X - 1, Y, Z) = 1;
                        getWireInfo(X - 1, Y, Z).ComponentSpace = 1;
                    }
                    if (X < D.Width - 1) {
                        getUsedSpace(X + 1, Y, Z) = 1;
                        getWireInfo(X + 1, Y, Z).ComponentSpace = 1;
                    }
                    if (Y > 0) {
                        getUsedSpace(X, Y - 1, Z) = 1;
                        getWireInfo(X, Y - 1, Z).ComponentSpace = 1;
                    }
                    if (Y < D.Height - 1) {
                        getUsedSpace(X, Y + 1, Z) = 1;
                        getWireInfo(X, Y + 1, Z).ComponentSpace = 1;
                    }
                    if (Z > 0) {
                        getUsedSpace(X, Y, Z - 1) = 1;
                        getWireInfo(X, Y, Z - 1).ComponentSpace = 1;
                    }
                    if (Z < D.Length - 1) {
                        getUsedSpace(X, Y, Z + 1) = 1;
                        getWireInfo(X, Y, Z + 1).ComponentSpace = 1;
                    }
                }
            }
        }
    }
    for (const auto& C : Connections) {
        const auto& [SourceCPtrs, SourcePort] = C->getSourcePortConnection();
        initPortUsedSpace(SourceCPtrs->getPinLocation(SourcePort), SourceCPtrs->getPinFacing(SourcePort));
        for (const auto& [SourceCPtrs, SourcePort] : C->getSinkPortConnections()) {
            initPortUsedSpace(SourceCPtrs->getPinLocation(SourcePort), SourceCPtrs->getPinFacing(SourcePort));
        }
    }
}

bool Router::regularRoute(Connection& C) {
    bool retFlag = true;
    // implement dijkstra
    Router::coord start;
    Router::coord congestionP;
    congestionP.x = -1;
    congestionP.y = -1;
    congestionP.z = -1;
    std::vector<Router::coord> end;
    std::tuple<uint16_t, uint16_t, uint16_t> startPin;
    // std::tuple<uint16_t, uint16_t, uint16_t> endPin;

    const auto& SourcePortConnection = C.getSourcePortConnection();
    const auto& SinkPortConnection = C.getSinkPortConnections();
    // source
    auto SrcFacing = ((SourcePortConnection).first)->getPinFacing((SourcePortConnection).second);
    startPin = ((SourcePortConnection).first)->getPinLocation((SourcePortConnection).second);
    // std::cout<<"wire: "<<C.getName()<<" startport name is "<<SourcePortConnection.second<<std::endl;
    start = Router::updateSinglePortUsedSpace(startPin, SrcFacing, congestionP);
    if (start.x == std::get<0>(startPin) && start.y == std::get<1>(startPin) && start.z == std::get<2>(startPin)) {
        std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z
                  << " because of other wires routing" << std::endl;
        std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                  << std::endl;
        if (!reRouteStartRouting(congestionP)) {
            return false;
        }
        // call update again TODO not good
        start = Router::updateSinglePortUsedSpace(startPin, SrcFacing, congestionP);
    }
    // int i = 0;
    for (auto it = SinkPortConnection.begin(); it != SinkPortConnection.end(); ++it) {
        // i++;
        // std::cout<<i<<" endport name is "<<it->second<<std::endl;
        auto temp = it->first->getPinLocation(it->second);
        auto endFace = (it->first)->getPinFacing(it->second);

        auto temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);

        if (temp_result.x == std::get<0>(temp) && temp_result.y == std::get<1>(temp) &&
            temp_result.z == std::get<2>(temp)) {
            std::cout << "FAIL routing to " << temp_result.x << ", " << temp_result.y << ", " << temp_result.z
                      << " because of other wires routing" << std::endl;
            std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                      << std::endl;
            if (!reRouteStartRouting(congestionP)) {
                retFlag = false;
                temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);
            } else {
                temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);
                end.push_back(temp_result);
            }
        } else {
            end.push_back(temp_result);
        }
    }

    std::vector<Router::coord> endTemp = end;
    std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare> Q;
    getPoint(start.x, start.y, start.z).cost = 0;
    getPoint(start.x, start.y, start.z).length = 1;
    getPoint(start.x, start.y, start.z).inserted = 1;
    Q.push(&getPoint(start.x, start.y, start.z));
    bool done = 0;
    while (!Q.empty()) {
        Router::Point* TempP = Q.top();
        Q.pop();
        TempP->visited = 1;
        for (auto it = endTemp.begin(); it != endTemp.end(); ++it) {
            if ((TempP->Loc.x == it->x) && (TempP->Loc.y == it->y) && (TempP->Loc.z == it->z)) {
                // std::cout <<"Location: "<<TempP->Loc.x<<" "<<TempP->Loc.y<<" "<<TempP->Loc.z<<std::endl;
                endTemp.erase(it);
                break;
            }
        }
        if (endTemp.empty())
            break;

        if (TempP->Loc.x /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/ &&
            (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.x < D.Width - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/
            && (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.z /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/ &&
            (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, Q, TempP);
        }
        if (TempP->Loc.z < D.Length - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/
            && (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, Q, TempP);
        }

        if (TempP->Loc.x && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y - 1, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.x && (TempP->Loc.y < D.Height - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y + 1, TempP->Loc.z, Q, TempP);
        }

        if ((TempP->Loc.x < D.Width - 1) && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y - 1, TempP->Loc.z, Q, TempP);
        }
        if ((TempP->Loc.x < D.Width - 1) && (TempP->Loc.y < D.Height - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y + 1, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.z && (TempP->Loc.y > 1) /* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z - 1, Q, TempP);
        }
        if (TempP->Loc.z && (TempP->Loc.y < D.Height - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z - 1, Q, TempP);
        }

        if ((TempP->Loc.z < D.Length - 1) && (TempP->Loc.y > 1) /* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z + 1, Q, TempP);
        }
        if ((TempP->Loc.z < D.Length - 1) && (TempP->Loc.y < D.Height - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z + 1, Q, TempP);
        }
    }
    C.setRouted(0);
    Point* ptr = NULL;
    int sinkId = -1;
    for (auto it : end) {
        sinkId++; // for matching added repeater with sink port
        if (getPoint(it.x, it.y, it.z).P) {
            C.setRouted(1);
            uint16_t TempX, TempY, TempZ;
            TempX = it.x;
            TempY = it.y;
            TempZ = it.z;
            if (getPoint(it.x, it.y, it.z).length >= MAX_NUM_OF_WIRE + 1) {
                DOUT(<< "Routing failed: Repeater is placed at the endpin " << TempX << " " << TempY << " " << TempZ
                     << std::endl);
                DOUT(<< "Try Routing the repeater ..." << std::endl);
                routingLastRepeater(&getPoint(it.x, it.y, it.z));
            }
            C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                     D.CellLib.getCellPtr("WIRE"), getPoint(it.x, it.y, it.z).ori));
            // WI[TempX][TempY][TempZ].CPtrs = &C;
            ptr = getPoint(it.x, it.y, it.z).P;
            while (ptr != NULL) {
                uint16_t TempX, TempY, TempZ;
                TempX = ptr->Loc.x;
                TempY = ptr->Loc.y;
                TempZ = ptr->Loc.z;
                if (ptr->length >= MAX_NUM_OF_WIRE + 1) {
                    // for(auto t: end){
                    //     if(TempX == t.x && TempY == t.y && TempZ == t.z){
                    //         std::cout<<"in while HITHITHIT "<<t.x<<" "<<t.y<<" "<<t.z<<std::endl;
                    //         break;
                    //     }
                    // }
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("BUF"), ptr->ori, sinkId));
                    // std::cout<<"repeater: "<<static_cast<int>(ptr->ori)<<std::endl;
                } else {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("WIRE"), ptr->ori));
                }
                // WI[TempX][TempY][TempZ].CPtrs = &C;
                ptr = ptr->P;
            }
        } else {
            DOUT(<< "FAIL routing from " << start.x << ", " << start.y << ", " << start.z << " to " << it.x << ", "
                 << it.y << ", " << it.z << std::endl);
            FailedWireSingleRouting = &C;
            retFlag = false;
            if (C.getUnableRouting() == 0) {
                C.setUnableRouting(1);
            } else if (C.getUnableRouting() == 2) {
                C.setUnableRouting(3);
            }
            C.Result.clear();
            C.RouteResult.clear();
            return retFlag;
        }
    }

    auto IllegalPoints = C.checkRouteResult();
    if (!IllegalPoints.empty()) {
        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>> congestionPoints_prev;
        return reRouteIllegal(C, IllegalPoints, congestionPoints_prev);
    }
    updateUsedSpace(C);
    // for (auto itt = C.Result.begin(); itt != C.Result.end(); ++itt) {
    //     if(C.getName()=="a[2]"){
    //        // std::cout << "start point is " << start.x << ", " << start.y << ", " << start.z << std::endl;
    //         std::cout << "failed routing " << std::get<0>(itt->coord) << ", " << std::get<1>(itt->coord) << ", "
    //                   << std::get<2>(itt->coord)<<std::endl;
    //     // if (std::get<0>(itt->coord) == 9 && std::get<1>(itt->coord) == 0 && std::get<2>(itt->coord) == 24)
    //     //     std::cout << "routing" << std::get<0>(itt->coord) << ", " << std::get<1>(itt->coord) << ", "
    //     //               << std::get<2>(itt->coord);
    //     //
    //     if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::ZeroCW)
    //     // std::cout<<"ORI:"<<0<<std::endl;
    //     //
    //     if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::OneCW)
    //     // std::cout<<"ORI:"<<1<<std::endl;
    //     //
    //     if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::TwoCW)
    //     // std::cout<<"ORI:"<<2<<std::endl;
    //     //
    //     if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::ThreeCW)
    //     // std::cout<<"ORI:"<<3<<std::endl;
    //     // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].length >= MAX_NUM_OF_WIRE
    //     +
    //     // 1){
    //     //    std::cout<<"HIT!!!"<<3<<std::endl;
    //     // }
    //      }else{break;}
    // }
    return retFlag;
}

bool Router::reRouteIllegal(
    Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints_prev) {
    std::string rollback_wire_name = C.getName();
    DOUT(<< "Wire " << rollback_wire_name << " routing illegal: " << std::endl);
    // if(C.getName() == "$abc$156$new_n28_"){
    //     for (auto& test_t: C.Result){
    //         std::cout<<"point list "<<std::get<0>(test_t.coord)<<" "<<std::get<1>(test_t.coord)<<"
    //         "<<std::get<2>(test_t.coord)<<std::endl;
    //     }
    // }
    // TODO:can be optimaze its code
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>> local_congestion_points;
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>> inserted_congestion_points;
    for (auto& i : congestionPoints) {
        DOUT(<< "    " << std::get<0>(i) << ", " << std::get<1>(i) << ", " << std::get<2>(i)
             << " Type: " << std::get<3>(i) << std::endl);
        bool temp_flag = false;
        for (auto& it : congestionPoints_prev) {
            if ((std::get<0>(it) == std::get<0>(i)) && (std::get<1>(it) == std::get<1>(i)) &&
                (std::get<2>(it) == std::get<2>(i))) {
                if (std::get<3>(it) == 1) {
                    std::cout << "        get it 3 times: rerouting for illegal failed" << std::endl;
                    C.Result.clear();
                    C.RouteResult.clear();
                    return false;
                }
            }
            if ((std::get<0>(it) == std::get<0>(i)) && (std::get<1>(it) == std::get<1>(i) + std::get<3>(i) + 1) &&
                (std::get<2>(it) == std::get<2>(i))) {
                if (std::get<3>(it) == 0) {
                    temp_flag = true;
                    std::cout << "        get it twice: disable the lower point for illegal" << std::endl;
                    if (getUsedSpace(std::get<0>(it), std::get<1>(it), std::get<2>(it)) == 3) {
                        getUsedSpace(std::get<0>(it), std::get<1>(it), std::get<2>(it)) = 0;
                        std::cout << "        get it twice: recover it: " << std::get<0>(it) << " " << std::get<1>(it)
                                  << " " << std::get<2>(it) << std::endl;
                    }
                    congestionPoints_prev.erase(it);
                    congestionPoints_prev.insert(
                        std::make_tuple(std::get<0>(i), std::get<1>(i), std::get<2>(i), 1, std::get<3>(i)));
                    inserted_congestion_points.insert(std::make_tuple(std::get<0>(i), std::get<1>(i), std::get<2>(i)));
                    // TODO should not break, need to traverse all points then quit
                    break;
                }
            }
        }
        if (!temp_flag) {
            inserted_congestion_points.insert(
                std::make_tuple(std::get<0>(i), std::get<1>(i) + std::get<3>(i) + 1, std::get<2>(i)));
            local_congestion_points.insert(std::make_tuple(std::get<0>(i), std::get<1>(i) + std::get<3>(i) + 1,
                                                           std::get<2>(i), 0, std::get<3>(i)));
        }
    }
    for (auto it : congestionPoints_prev) {
        // std::cout << "    prev " << std::get<0>(it) << ", " << std::get<1>(it) << ", " << std::get<2>(it)<< "
        // converted_type: " << std::get<3>(it) << std::endl;
        local_congestion_points.insert(it);
    }
    // for(auto it = local_congestion_points.begin(); it!=local_congestion_points.end(); ++it){
    //     std::cout <<"  NOW illegal points  "<<std::get<0>(*it)<<", "<<std::get<1>(*it)<<", "<<std::get<2>(*it)<<" ---
    //     "<<std::get<3>(*it)<<" illegal type: "<<std::get<4>(*it)<<std::endl; std::cout <<"      NOW illegal points
    //     Usedspace "<<UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)]<<std::endl;
    // }
    return reRouteIllegalHelper(C, local_congestion_points, inserted_congestion_points);
}

bool Router::reRouteIllegalHelper(
    Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& local_congestion_points,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& inserted_congestion_points) {
    std::string rollback_wire_name = C.getName();
    bool break_loop = false;

    do {
        for (auto it = local_congestion_points.begin(); it != local_congestion_points.end(); ++it) {
            // TODO what if usedspace is 2?
            // std::cout<<"to set to 3: "<<std::get<0>(*it)<<" "<<std::get<1>(*it)<<" "<<std::get<2>(*it)<<" used
            // "<<UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)]<<std::endl;
            if (getUsedSpace(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)) != 1) {
                getUsedSpace(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)) = 3;
            }
        }
        std::for_each(Points.begin(), Points.end(), [this](Point& P) {
            P.cost = D.Width * D.Height * D.Length;
            P.length = -1;
            P.ori = HDL2Redstone::Orientation::ZeroCW;
            P.P = nullptr;
            P.visited = 0;
            P.inserted = 0;
        });
        C.Result.clear();
        C.RouteResult.clear();
        DOUT(<< "ReRouting for Illegal: " << rollback_wire_name << std::endl);
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>> RetIllegalPoints;
        bool ReRouteFlag = reRouteforIllegalRegularRouteHelper(C, RetIllegalPoints, local_congestion_points,
                                                               inserted_congestion_points);
        for (auto it = local_congestion_points.begin(); it != local_congestion_points.end(); ++it) {
            if (getUsedSpace(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)) == 3) {
                // TODO what if original usedspace is 2?
                getUsedSpace(std::get<0>(*it), std::get<1>(*it), std::get<2>(*it)) = 0;
            }
        }
        if (ReRouteFlag) {
            DOUT(<< "ReRouting for Illegal Success: " << rollback_wire_name << std::endl);
            return true;
        } else {
            if (RetIllegalPoints.empty()) {
                DOUT(<< "ReRouting for Illegal Failed caused by other reasons: " << rollback_wire_name << std::endl);
                C.Result.clear();
                C.RouteResult.clear();
                return false;
            }
            // for(auto it: local_congestion_points){
            //     std::cout << "local_congestion_points start end pin illegal points: " << std::get<0>(it)<<"
            //     "<<std::get<1>(it)<<" "<<std::get<2>(it)<<" -> "<<std::get<3>(it)<<" "<<std::get<4>(it)<< std::endl;
            // }
            // for(auto it: RetIllegalPoints){
            //     std::cout << "RetIllegalPoints start end pin illegal points: " << std::get<0>(it)<<"
            //     "<<std::get<1>(it)<<" "<<std::get<2>(it)<< std::endl;
            // }
            bool temp_reroute_flag = false;
            std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>> local_congestion_points_temp;
            for (auto nIterator = local_congestion_points.begin(); nIterator != local_congestion_points.end();
                 nIterator++) {
                bool already_inserted = false;
                for (auto mIterator = RetIllegalPoints.begin(); mIterator != RetIllegalPoints.end(); mIterator++) {
                    if ((std::get<0>(*nIterator) == std::get<0>(*mIterator)) &&
                        (std::get<1>(*nIterator) == std::get<1>(*mIterator)) &&
                        (std::get<2>(*nIterator) == std::get<2>(*mIterator))) {
                        if (std::get<3>(*nIterator) == 2) {
                            DOUT(<< "ReRouting for Illegal Failed caused by start or end pin: " << rollback_wire_name
                                 << std::endl);
                            C.Result.clear();
                            C.RouteResult.clear();
                            return false;
                        }
                        if (getUsedSpace(std::get<0>(*nIterator), std::get<1>(*nIterator), std::get<2>(*nIterator)) ==
                            3) {
                            getUsedSpace(std::get<0>(*nIterator), std::get<1>(*nIterator), std::get<2>(*nIterator)) = 0;
                        }
                        local_congestion_points_temp.insert(std::make_tuple(
                            std::get<0>(*nIterator), std::get<1>(*nIterator) - (std::get<4>(*nIterator) + 1),
                            std::get<2>(*nIterator), std::get<3>(*nIterator) + 1, std::get<4>(*nIterator)));
                        temp_reroute_flag = true;
                        already_inserted = true;
                    } else {
                        // do nothing
                    }
                }
                if (!already_inserted) {
                    local_congestion_points_temp.insert(*nIterator);
                }
            }
            local_congestion_points = local_congestion_points_temp;
            // for(auto it: local_congestion_points){
            //     std::cout << "start end pin illegal points: " << std::get<0>(it)<<" "<<std::get<1>(it)<<"
            //     "<<std::get<2>(it)<<" "<<std::get<3>(it)<<" "<<std::get<4>(it)<< std::endl;
            // }
            DOUT(<< "ReRouting for Illegal Failed: " << rollback_wire_name << ", Still Trying ..." << std::endl);
            break_loop = !temp_reroute_flag;
        }
    } while (!break_loop);
    return false;
}

bool Router::reRouteStartRouting(coord congestionPoint) {
    auto& CPtrs = getWireInfo(congestionPoint.x, congestionPoint.y, congestionPoint.z).CPtrs;
    if (CPtrs.empty()) {
        std::cout << "ReRoute Failed: Never Reach Here!" << std::endl;
        return false;
    }
    std::set<std::string> rollback_wire_name;
    // roll back congestion wires
    for (auto& it : CPtrs) {
        rollback_wire_name.insert(it->getName());
        std::cout << it->getName() << " is rolling back for ReRouting" << std::endl;
        it->Result.clear();
        it->RouteResult.clear();
    }
    // roll back congestion wires
    for (uint16_t X = 0; X != D.Width; ++X) {
        for (uint16_t Y = 0; Y != D.Height; ++Y) {
            for (uint16_t Z = 0; Z != D.Length; ++Z) {
                auto& CurCPtrs = getWireInfo(X, Y, Z).CPtrs;
                if (!CurCPtrs.empty()) {
                    for (auto It = CurCPtrs.begin(); It != CurCPtrs.end();) {
                        if (rollback_wire_name.count((*It)->getName())) {
                            It = CurCPtrs.erase(It);
                        } else {
                            ++It;
                        }
                    }
                    if (getWireInfo(X, Y, Z).ComponentSpace != 1 && CurCPtrs.empty()) {
                        getUsedSpace(X, Y, Z) = 0;
                    }
                }
            }
        }
    }
    // force congestion point is occupied
    getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z) = 3;
    if (congestionPoint.y < D.Height - 1) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y + 1, congestionPoint.z) == 0) {
            getUsedSpace(congestionPoint.x, congestionPoint.y + 1, congestionPoint.z) = 3;
        }
    }
    if (congestionPoint.y > 0) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y - 1, congestionPoint.z) == 0) {
            getUsedSpace(congestionPoint.x, congestionPoint.y - 1, congestionPoint.z) = 3;
        }
    }
    if (congestionPoint.x > 0) {
        if (getUsedSpace(congestionPoint.x - 1, congestionPoint.y, congestionPoint.z) == 0) {
            getUsedSpace(congestionPoint.x - 1, congestionPoint.y, congestionPoint.z) = 3;
        }
    }
    if (congestionPoint.x < D.Width - 1) {
        if (getUsedSpace(congestionPoint.x + 1, congestionPoint.y, congestionPoint.z) == 0) {
            getUsedSpace(congestionPoint.x + 1, congestionPoint.y, congestionPoint.z) = 3;
        }
    }
    if (congestionPoint.z > 0) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z - 1) == 0) {
            getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z - 1) = 3;
        }
    }
    if (congestionPoint.z < D.Length - 1) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z + 1) == 0) {
            getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z + 1) = 3;
        }
    }
    bool ReRouteFlag = true;
    for (auto it : CPtrs) {
        // route congested wire again
        std::for_each(Points.begin(), Points.end(), [this](Point& P) {
            P.cost = D.Width * D.Height * D.Length;
            P.length = -1;
            P.ori = HDL2Redstone::Orientation::ZeroCW;
            P.P = nullptr;
            P.visited = 0;
            P.inserted = 0;
        });
        std::cout << "ReRouting: " << it->getName() << std::endl;
        if (!regularRoute(*it)) {
            std::cout << "ReRouting Failed: " << it->getName() << std::endl;
            ReRouteFlag = false;
        } else {
            std::cout << "ReRouting Success: " << it->getName() << std::endl;
        }
    }
    if (getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z) == 3) {
        getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z) = 0;
    }
    if (congestionPoint.y < D.Height - 1) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y + 1, congestionPoint.z) == 3) {
            getUsedSpace(congestionPoint.x, congestionPoint.y + 1, congestionPoint.z) = 0;
        }
    }
    if (congestionPoint.y > 0) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y - 1, congestionPoint.z) == 3) {
            getUsedSpace(congestionPoint.x, congestionPoint.y - 1, congestionPoint.z) = 0;
        }
    }
    if (congestionPoint.x > 0) {
        if (getUsedSpace(congestionPoint.x - 1, congestionPoint.y, congestionPoint.z) == 3) {
            getUsedSpace(congestionPoint.x - 1, congestionPoint.y, congestionPoint.z) = 0;
        }
    }
    if (congestionPoint.x < D.Width - 1) {
        if (getUsedSpace(congestionPoint.x + 1, congestionPoint.y, congestionPoint.z) == 3) {
            getUsedSpace(congestionPoint.x + 1, congestionPoint.y, congestionPoint.z) = 0;
        }
    }
    if (congestionPoint.z > 0) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z - 1) == 3) {
            getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z - 1) = 0;
        }
    }
    if (congestionPoint.z < D.Length - 1) {
        if (getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z + 1) == 3) {
            getUsedSpace(congestionPoint.x, congestionPoint.y, congestionPoint.z + 1) = 0;
        }
    }
    std::for_each(Points.begin(), Points.end(), [this](Point& P) {
        P.cost = D.Width * D.Height * D.Length;
        P.length = -1;
        P.ori = HDL2Redstone::Orientation::ZeroCW;
        P.P = nullptr;
        P.visited = 0;
        P.inserted = 0;
    });
    return ReRouteFlag;
}

bool Router::reRouteforIllegalRegularRouteHelper(
    Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& RetIllegalPoints,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& inserted_congestion_points) {
    bool retFlag = true;
    // implement dijkstra
    Router::coord start;
    Router::coord congestionP;
    congestionP.x = -1;
    congestionP.y = -1;
    congestionP.z = -1;
    std::vector<Router::coord> end;
    std::tuple<uint16_t, uint16_t, uint16_t> startPin;
    // std::tuple<uint16_t, uint16_t, uint16_t> endPin;

    auto SourcePortConnection_ = C.getSourcePortConnection();
    auto PortConnection_ = C.getSinkPortConnections();
    // source
    auto SrcFacing = ((SourcePortConnection_).first)->getPinFacing((SourcePortConnection_).second);
    startPin = ((SourcePortConnection_).first)->getPinLocation((SourcePortConnection_).second);
    // std::cout<<"wire: "<<C.getName()<<" startport name is "<<SourcePortConnection_.second<<std::endl;
    start = Router::updateSinglePortUsedSpace(startPin, SrcFacing, congestionP);
    if (start.x == std::get<0>(startPin) && start.y == std::get<1>(startPin) && start.z == std::get<2>(startPin)) {
        DOUT(<< "FAIL routing from " << start.x << ", " << start.y << ", " << start.z
             << " because of other wires routing" << std::endl);
        DOUT(<< "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z << std::endl);
        // TODO need to check congestionP.y and congestioP.y-1 two points
        if (getUsedSpace(congestionP.x, congestionP.y, congestionP.z) ==
            3) { // Here to check the congestion point is set as non-reachable, it should fail
            RetIllegalPoints.insert(std::make_tuple(congestionP.x, congestionP.y, congestionP.z));
            return false;
        } else if (getUsedSpace(congestionP.x, congestionP.y - 1, congestionP.z) == 3) {
            RetIllegalPoints.insert(std::make_tuple(congestionP.x, congestionP.y - 1, congestionP.z));
            return false;
        } else {
            if (!reRouteStartRouting(congestionP)) {
                return false;
            }
        }
        // call update again TODO not good
        start = Router::updateSinglePortUsedSpace(startPin, SrcFacing, congestionP);
    }
    // below are end points
    bool RetforCongestionFlag = false;
    // int i = 0;
    for (auto it = PortConnection_.begin(); it != PortConnection_.end(); ++it) {
        // i++;
        // std::cout<<i<<" endport name is "<<it->second<<std::endl;
        auto temp = it->first->getPinLocation(it->second);
        auto endFace = (it->first)->getPinFacing(it->second);
        auto temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);
        if (temp_result.x == std::get<0>(temp) && temp_result.y == std::get<1>(temp) &&
            temp_result.z == std::get<2>(temp)) {
            DOUT(<< "FAIL routing to " << temp_result.x << ", " << temp_result.y << ", " << temp_result.z
                 << " because of other wires routing" << std::endl);
            DOUT(<< "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                 << std::endl);
            if (getUsedSpace(congestionP.x, congestionP.y, congestionP.z) ==
                3) { // Here to check the congestion point is set as non-reachable, it should fail
                RetIllegalPoints.insert(std::make_tuple(congestionP.x, congestionP.y, congestionP.z));
                RetforCongestionFlag = true;
            } else if (getUsedSpace(congestionP.x, congestionP.y - 1, congestionP.z) == 3) {
                RetIllegalPoints.insert(std::make_tuple(congestionP.x, congestionP.y - 1, congestionP.z));
                RetforCongestionFlag = true;
            } else {
                if (!reRouteStartRouting(congestionP)) {
                    retFlag = false;
                    temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);
                } else {
                    temp_result = Router::updateSinglePortUsedSpace(temp, endFace, congestionP);
                    end.push_back(temp_result);
                }
            }
        } else {
            end.push_back(temp_result);
        }
    }
    if (RetforCongestionFlag) {
        return false;
    }

    std::vector<Router::coord> endTemp = end;
    std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare> Q;
    getPoint(start.x, start.y, start.z).cost = 0;
    getPoint(start.x, start.y, start.z).length = 1;
    getPoint(start.x, start.y, start.z).inserted = 1;
    Q.push(&getPoint(start.x, start.y, start.z));
    bool done = 0;

    while (!Q.empty()) {
        Router::Point* TempP = Q.top();
        Q.pop();
        TempP->visited = 1;
        for (auto it = endTemp.begin(); it != endTemp.end(); ++it) {
            if ((TempP->Loc.x == it->x) && (TempP->Loc.y == it->y) && (TempP->Loc.z == it->z)) {
                // std::cout <<"Location: "<<std::get<0>(TempP->Loc)<<" "<<std::get<1>(TempP->Loc)<<"
                // "<<std::get<2>(TempP->Loc)<<std::endl;
                endTemp.erase(it);
                break;
            }
        }
        if (endTemp.empty())
            break;

        if (TempP->Loc.x /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/ &&
            (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.x < D.Width - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/
            && (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.z /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/ &&
            (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, Q, TempP);
        }
        if (TempP->Loc.z < D.Length - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/
            && (TempP->Loc.y) % 2 == 1) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, Q, TempP);
        }

        if (TempP->Loc.x && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y - 1, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.x && (TempP->Loc.y < D.Height - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y + 1, TempP->Loc.z, Q, TempP);
        }

        if ((TempP->Loc.x < D.Width - 1) && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y - 1, TempP->Loc.z, Q, TempP);
        }
        if ((TempP->Loc.x < D.Width - 1) && (TempP->Loc.y < D.Height - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y + 1, TempP->Loc.z, Q, TempP);
        }
        if (TempP->Loc.z && (TempP->Loc.y > 1) /* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z - 1, Q, TempP);
        }
        if (TempP->Loc.z && (TempP->Loc.y < D.Height - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z - 1, Q, TempP);
        }

        if ((TempP->Loc.z < D.Length - 1) && (TempP->Loc.y > 1) /* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z + 1, Q, TempP);
        }
        if ((TempP->Loc.z < D.Length - 1) && (TempP->Loc.y < D.Height - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z + 1, Q, TempP);
        }
    }
    C.setRouted(0);
    Point* ptr = NULL;
    int sinkId = -1;
    for (auto it : end) {
        sinkId++; // for matching added repeater with sink port
        if (getPoint(it.x, it.y, it.z).P) {
            C.setRouted(1);
            uint16_t TempX, TempY, TempZ;
            TempX = it.x;
            TempY = it.y;
            TempZ = it.z;
            if (getPoint(it.x, it.y, it.z).length >= MAX_NUM_OF_WIRE + 1) {
                DOUT(<< "Routing failed: Repeater is placed at the endpin " << TempX << " " << TempY << " " << TempZ
                     << std::endl);
                DOUT(<< "Try Routing the repeater ..." << std::endl);
                routingLastRepeater(&getPoint(it.x, it.y, it.z));
            }
            C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                     D.CellLib.getCellPtr("WIRE"), getPoint(it.x, it.y, it.z).ori));
            // WI[TempX][TempY][TempZ].CPtrs = &C;
            ptr = getPoint(it.x, it.y, it.z).P;
            while (ptr != NULL) {
                uint16_t TempX, TempY, TempZ;
                TempX = ptr->Loc.x;
                TempY = ptr->Loc.y;
                TempZ = ptr->Loc.z;
                if (ptr->length >= MAX_NUM_OF_WIRE + 1) {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("BUF"), ptr->ori, sinkId));
                } else {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("WIRE"), ptr->ori));
                }
            next:;
                // WI[TempX][TempY][TempZ].CPtrs = &C;
                ptr = ptr->P;
            }
        } else {
            DOUT(<< "FAIL routing from " << start.x << ", " << start.y << ", " << start.z << " to " << it.x << ", "
                 << it.y << ", " << it.z << std::endl);
            RetIllegalPoints = inserted_congestion_points;
            FailedWireSingleRouting = &C;
            retFlag = false;
            if (C.getUnableRouting() == 0) {
                C.setUnableRouting(1);
            } else if (C.getUnableRouting() == 2) {
                C.setUnableRouting(3);
            }
            C.Result.clear();
            C.RouteResult.clear();
            return retFlag;
        }
    }
    auto IllegalPoints = C.checkRouteResult();
    if (!IllegalPoints.empty()) {
        return reRouteIllegal(C, IllegalPoints, congestionPoints);
    }
    updateUsedSpace(C);
    // for(auto itt = C.Result.begin(); itt!=C.Result.end(); ++itt)
    // {
    //     if(C.getName()=="x")
    //     std::cout << "routing"
    //                     <<std::get<0>(itt->coord)<<", "<<std::get<1>(itt->coord)<<",
    //                     "<<std::get<2>(itt->coord)<<std::endl;
    // }
    return retFlag;
}

void Router::checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z,
                              std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                              Router::Point*& TempP) {
    if (!getUsedSpace(x, y, z) && !getUsedSpace(x, y - 1, z)) {
        // check the node is in for its potential path
        // check up to +- 2 unit are clean
        if ((getPoint(x, y, z).cost >= (1 + getPoint(x, y, z).base_cost) * getPoint(x, y, z).mul_cost + TempP->cost)) {
            bool legal = 1;
            // // this while loop is checking self wire illegal situation
            // Router::Point* temp_ptr = TempP->P;
            // while (temp_ptr) {
            //     if ((temp_ptr->Loc.y < std::get<1>(Space) - 2) &&
            //         ((y == temp_ptr->Loc.y + 1) || (y == temp_ptr->Loc.y + 2)) && (temp_ptr->Loc.x == x) &&
            //         (temp_ptr->Loc.z == z)) {
            //         legal = 0;
            //         break;
            //     }
            //     if ((temp_ptr->Loc.y > 0) && ((y == temp_ptr->Loc.y - 1) || (y == temp_ptr->Loc.y - 2))&&
            //     (temp_ptr->Loc.x == x) &&
            //         (temp_ptr->Loc.z == z)) {
            //         legal = 0;
            //         break;
            //     }
            //     temp_ptr = temp_ptr->P;
            // }
            if (legal) {
                if (checkUpdateGraphHelper(TempP, &getPoint(x, y, z))) {
                    getPoint(x, y, z).cost =
                        (1 + getPoint(x, y, z).base_cost) * getPoint(x, y, z).mul_cost + TempP->cost;
                    getPoint(x, y, z).P = TempP;
                    if (!getPoint(x, y, z).visited && !getPoint(x, y, z).inserted) {
                        getPoint(x, y, z).inserted = 1;
                        Q.push(&getPoint(x, y, z));
                    }
                }
            }
        }
    }
}

bool Router::checkUpdateGraphHelper(Router::Point* Parent, Router::Point* Current) {
    bool RetFlag = true;
    auto ori_ = Current->ori;
    if (Parent->Loc.x > Current->Loc.x) {
        ori_ = HDL2Redstone::Orientation::TwoCW;
    } else if (Parent->Loc.x < Current->Loc.x) {
        ori_ = HDL2Redstone::Orientation::ZeroCW;
    } else if (Parent->Loc.z > Current->Loc.z) {
        ori_ = HDL2Redstone::Orientation::ThreeCW;
    } else if (Parent->Loc.z < Current->Loc.z) {
        ori_ = HDL2Redstone::Orientation::OneCW;
    }

    if (Parent->length == MAX_NUM_OF_WIRE + 1) {
        if (Parent->ori != ori_) {
            RetFlag = false;
        } else if (Parent->Loc.y != Current->Loc.y) {
            RetFlag = false;
        } else {
            if (Current->cost == (1 + Current->base_cost) * Current->mul_cost + Parent->cost) {
                if (Parent->Loc.y == Current->Loc.y) {
                    Current->length = 1;
                } else {
                    RetFlag = false;
                }
            } else {
                Current->length = 1;
            }
        }
    } else if (Parent->length == MAX_NUM_OF_WIRE) {
        if (Parent->Loc.y != Current->Loc.y) {
            RetFlag = false;
        } else {
            if (Current->cost == (1 + Current->base_cost) * Current->mul_cost + Parent->cost) {
                if (Parent->Loc.y == Current->Loc.y) {
                    Current->length = Parent->length + 1;
                } else {
                    RetFlag = false;
                }
            } else {
                Current->length = Parent->length + 1;
            }
        }
    } else {
        if (Current->cost == (1 + Current->base_cost) * Current->mul_cost + Parent->cost) {
            if ((Parent->Loc.y == Current->Loc.y) && (Current->P->Loc.y != Current->Loc.y)) {
                Current->length = Parent->length + 1;
            } else if ((Parent->Loc.y == Current->Loc.y) && (Current->P->Loc.y == Current->Loc.y)) {
                // 1 5 9 13; 3, 7, 11,15
                // if(Current->Loc.y % 4 == 3){
                if (ori_ == HDL2Redstone::Orientation::ZeroCW || ori_ == HDL2Redstone::Orientation::TwoCW) {
                    Current->length = Parent->length + 1;
                } else {
                    if (Current->ori == HDL2Redstone::Orientation::ZeroCW ||
                        Current->ori == HDL2Redstone::Orientation::TwoCW) {
                        RetFlag = false;
                    } else {
                        Current->length = Parent->length + 1;
                    }
                }
                // }else{
                //     if (ori_ == HDL2Redstone::Orientation::OneCW || ori_ == HDL2Redstone::Orientation::ThreeCW) {
                //         Current->length = Parent->length + 1;
                //     } else {
                //         if (Current->ori == HDL2Redstone::Orientation::OneCW || Current->ori ==
                //         HDL2Redstone::Orientation::ThreeCW) {
                //             RetFlag = false;
                //         } else {
                //             Current->length = Parent->length + 1;
                //         }
                //     }
                // }
            } else if ((Parent->Loc.y != Current->Loc.y) && (Current->P->Loc.y != Current->Loc.y)) {
                Current->length = Parent->length + 1;
            } else {
                RetFlag = false;
            }
        } else {
            Current->length = Parent->length + 1;
        }
    }
    if (RetFlag) {
        Current->ori = ori_;
    }
    return RetFlag;
}

bool Router::checkandKeepResult(Connection& C) {
    bool RetFlag = true;
    // if(C.getName()=="$abc$73$new_n14_"){
    //              std::cout<<"\nstart1FOUND wire status: "<<UsedSpace[24][13][9]<<"
    //              "<<WI[24][13][9].ComponentSpace<<std::endl; if(!WI[24][13][9].CPtrs.empty()){
    //                  for (auto it: WI[24][13][9].CPtrs)
    //                 std::cout<<"\nstart1FOUND wire name: "<<it->getName()<<std::endl;
    //              }else{
    //                  std::cout<<"\nstart1FOUND wire EMPTY"<<std::endl;
    //              }
    //         if (C.Result.empty()) {std::cout<<"\nstart1FOUND result EMPTY"<<std::endl;}
    // }
    if (!C.Result.empty()) {
        for (const auto& R : C.Result) {
            if (getUsedSpace(std::get<0>(R.coord), std::get<1>(R.coord), std::get<2>(R.coord)) == 1 ||
                getUsedSpace(std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord)) == 1) {
                std::cout << C.getName() << " is reseted" << std::endl;
                RetFlag = false;
                break;
            }
        }
    } else {
        return false;
    }
    if (!RetFlag) {
        C.setRouted(0);
        // roll back congestion wires
        for (uint16_t X = 0; X != D.Width; ++X) {
            for (uint16_t Y = 0; Y != D.Height; ++Y) {
                for (uint16_t Z = 0; Z != D.Length; ++Z) {
                    auto& CurCPtrs = getWireInfo(X, Y, Z).CPtrs;
                    if (!CurCPtrs.empty()) {
                        for (auto It = CurCPtrs.begin(); It != CurCPtrs.end();) {
                            if (C.getName() == (*It)->getName()) {
                                It = CurCPtrs.erase(It);
                            } else {
                                ++It;
                            }
                        }
                        if (getWireInfo(X, Y, Z).ComponentSpace != 1 && CurCPtrs.empty()) {
                            getUsedSpace(X, Y, Z) = 0;
                        }
                    }
                }
            }
        }
        C.Result.clear();
        C.RouteResult.clear();
        //  std::cout<<"\nSTARTFOUND wire status: "<<UsedSpace[12][1][1]<<"   "<<WI[12][1][1].ComponentSpace<<std::endl;
        //  if(!WI[12][1][1].CPtrs.empty()){
        //      for (auto it: WI[12][1][1].CPtrs)
        //     std::cout<<"\nSTARTFOUND wire name: "<<it->getName()<<std::endl;
        //  }else{
        //      std::cout<<"\nSTARTFOUND wire EMPTY"<<std::endl;
        //  }
    } else {
        // update usedspace and WI
        updateUsedSpace(C);
    }
    return RetFlag;
}

bool Router::checkKeepOrUpdate(Connection& C) {
    bool RetFlag = true;
    if (!C.Result.empty()) {
        for (const auto& R : C.Result) {
            if (getUsedSpace(std::get<0>(R.coord), std::get<1>(R.coord), std::get<2>(R.coord)) == 1 ||
                getUsedSpace(std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord)) == 1) {
                getPoint(std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord)).mul_cost++;
                DOUT(<< C.getName() << " is reseted. " << std::get<0>(R.coord) << " " << std::get<1>(R.coord) << " "
                     << std::get<2>(R.coord) << " mul_cost increases to "
                     << getPoint(std::get<0>(R.coord), std::get<1>(R.coord) + 1, std::get<2>(R.coord)).mul_cost
                     << std::endl);
                RetFlag = false;
            }
        }
    } else {
        return false;
    }
    if (!RetFlag) {
        C.setRouted(0);
        // roll back congestion wires
        for (uint16_t X = 0; X != D.Width; ++X) {
            for (uint16_t Y = 0; Y != D.Height; ++Y) {
                for (uint16_t Z = 0; Z != D.Length; ++Z) {
                    auto& CurCPtrs = getWireInfo(X, Y, Z).CPtrs;
                    if (!CurCPtrs.empty()) {
                        for (auto It = CurCPtrs.begin(); It != CurCPtrs.end();) {
                            if (C.getName() == (*It)->getName()) {
                                It = CurCPtrs.erase(It);
                            } else {
                                ++It;
                            }
                        }
                        if (getWireInfo(X, Y, Z).ComponentSpace != 1 && CurCPtrs.empty()) {
                            getUsedSpace(X, Y, Z) = 0;
                        }
                    }
                }
            }
        }
        C.Result.clear();
        C.RouteResult.clear();
    } else {
        // update usedspace and WI
        updateUsedSpace(C);
    }
    return RetFlag;
}

void Router::initPortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac) {
    Coordinate Coord{.X = std::get<0>(Loc), .Y = std::get<1>(Loc), .Z = std::get<2>(Loc)};
    if (Fac == Facing::North) {
        getWireInfo(Coord.X, Coord.Y, Coord.Z - 1).ComponentSpace = 2;
        getWireInfo(Coord.X, Coord.Y - 1, Coord.Z - 1).ComponentSpace = 2;
        Coord.Z--;
    } else if (Fac == Facing::East) {
        getWireInfo(Coord.X + 1, Coord.Y, Coord.Z).ComponentSpace = 2;
        getWireInfo(Coord.X + 1, Coord.Y - 1, Coord.Z).ComponentSpace = 2;
        Coord.X++;
    } else if (Fac == Facing::South) {
        getWireInfo(Coord.X, Coord.Y, Coord.Z + 1).ComponentSpace = 2;
        getWireInfo(Coord.X, Coord.Y - 1, Coord.Z + 1).ComponentSpace = 2;
        Coord.Z++;
    } else {
        getWireInfo(Coord.X - 1, Coord.Y, Coord.Z).ComponentSpace = 2;
        getWireInfo(Coord.X - 1, Coord.Y - 1, Coord.Z).ComponentSpace = 2;
        Coord.X--;
    }
    initPortUsedSpaceHelper(Coord);
}

void Router::initPortUsedSpaceHelper(const Coordinate& Coord) {
    getUsedSpace(Coord.X, Coord.Y, Coord.Z) = 2;
    getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z) = 2;
    if (Coord.X > 0) {
        getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) = (getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) != 1) ? 2 : 1;
        getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) =
            (getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) != 1) ? 2 : 1;
    }
    if (Coord.X < D.Width - 1) {
        getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) = (getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) != 1) ? 2 : 1;
        getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) =
            (getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) != 1) ? 2 : 1;
    }
    if (Coord.Z > 0) {
        getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) = (getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) != 1) ? 2 : 1;
        getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) =
            (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) != 1) ? 2 : 1;
    }
    if (Coord.Z < D.Length - 1) {
        getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) = (getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) != 1) ? 2 : 1;
        getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) =
            (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) != 1) ? 2 : 1;
    }
}

void Router::updateUsedSpace(Connection& C) {
    std::set<Connection::ConnectionResult, Connection::resultcomp>& Result = C.Result;
    // std::cout << "route result" << std::endl;
    for (const auto& entry : Result) {
        Router::coord entry_temp;
        entry_temp.x = std::get<0>(entry.coord);
        entry_temp.y = std::get<1>(entry.coord);
        entry_temp.z = std::get<2>(entry.coord);
        /////	//temp
        // std::cout<<"coord: "<<entry_temp.x<<" "<<entry_temp.y<<" "<<entry_temp.z<<std::endl;
        // std::cout<<"cell: "<<*entry.CellPtr;
        // std::cout<<"sink id:"<<entry.SinkId<<std::endl;
        // std::cout<<"source:"<<C.getSourcePortConnection().first<<" sink
        // ports:"<<C.getSinkPortConnections().size()<<std::endl;
        getUsedSpace(entry_temp.x, entry_temp.y + 1, entry_temp.z) = 1;
        getWireInfo(entry_temp.x, entry_temp.y + 1, entry_temp.z).CPtrs.insert(&C);
        getUsedSpace(entry_temp.x, entry_temp.y, entry_temp.z) = 1;
        getWireInfo(entry_temp.x, entry_temp.y, entry_temp.z).CPtrs.insert(&C);
        // if ((entry_temp.y > 0)) {
        //     UsedSpace[entry_temp.x][entry_temp.y - 1][entry_temp.z] = 1;
        //     WI[entry_temp.x][entry_temp.y - 1][entry_temp.z].CPtrs.insert(&C);
        // }
        // if ((entry_temp.y < std::get<1>(Space) - 2)) {
        //     UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z] = 1;
        //     WI[entry_temp.x][entry_temp.y + 2][entry_temp.z].CPtrs.insert(&C);
        // }
        if (entry_temp.x > 0) {
            getUsedSpace(entry_temp.x - 1, entry_temp.y + 1, entry_temp.z) = 1;
            getWireInfo(entry_temp.x - 1, entry_temp.y + 1, entry_temp.z).CPtrs.insert(&C);
            getUsedSpace(entry_temp.x - 1, entry_temp.y, entry_temp.z) = 1;
            getWireInfo(entry_temp.x - 1, entry_temp.y, entry_temp.z).CPtrs.insert(&C);
            // if ((entry_temp.y < std::get<1>(Space) - 2)) {
            //     UsedSpace[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z] = 1;
            //     WI[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z].CPtrs.insert(&C);
            // }
        }
        if (entry_temp.x < D.Width - 1) {
            getUsedSpace(entry_temp.x + 1, entry_temp.y + 1, entry_temp.z) = 1;
            getWireInfo(entry_temp.x + 1, entry_temp.y + 1, entry_temp.z).CPtrs.insert(&C);
            getUsedSpace(entry_temp.x + 1, entry_temp.y, entry_temp.z) = 1;
            getWireInfo(entry_temp.x + 1, entry_temp.y, entry_temp.z).CPtrs.insert(&C);
            // if ((entry_temp.y < std::get<1>(Space) - 2)) {
            //     UsedSpace[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z] = 1;
            //     WI[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z].CPtrs.insert(&C);
            // }
        }
        if (entry_temp.z > 0) {
            getUsedSpace(entry_temp.x, entry_temp.y + 1, entry_temp.z - 1) = 1;
            getWireInfo(entry_temp.x, entry_temp.y + 1, entry_temp.z - 1).CPtrs.insert(&C);
            getUsedSpace(entry_temp.x, entry_temp.y, entry_temp.z - 1) = 1;
            getWireInfo(entry_temp.x, entry_temp.y, entry_temp.z - 1).CPtrs.insert(&C);
            // if ((entry_temp.y < std::get<1>(Space) - 2)) {
            //     UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1] = 1;
            //     WI[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1].CPtrs.insert(&C);
            // }
        }
        if (entry_temp.z < D.Length - 1) {
            getUsedSpace(entry_temp.x, entry_temp.y + 1, entry_temp.z + 1) = 1;
            getWireInfo(entry_temp.x, entry_temp.y + 1, entry_temp.z + 1).CPtrs.insert(&C);
            getUsedSpace(entry_temp.x, entry_temp.y, entry_temp.z + 1) = 1;
            getWireInfo(entry_temp.x, entry_temp.y, entry_temp.z + 1).CPtrs.insert(&C);
            // if ((entry_temp.y < std::get<1>(Space) - 2)) {
            //     UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1] = 1;
            //     WI[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1].CPtrs.insert(&C);
            // }
        }
    }
}

Router::coord Router::updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac,
                                                coord& congestion) {
    Coordinate Coord{.X = std::get<0>(Loc), .Y = std::get<1>(Loc), .Z = std::get<2>(Loc)};
    congestion.x = std::get<0>(Loc);
    congestion.y = std::get<1>(Loc);
    congestion.z = std::get<2>(Loc);
    bool Temp = false;
    if (Fac == Facing::North) {
        congestion.z--;
        if (getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) != 1 && getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) != 3 &&
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) != 1 &&
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) != 3) {
            getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) = 0;
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) = 0;
            Coord.Z--;
            Temp = true;
        }
    } else if (Fac == Facing::East) {
        congestion.x++;
        if (getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) != 1 && getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) != 3 &&
            getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) != 1 &&
            getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) != 3) {
            getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) = 0;
            getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) = 0;
            Coord.X++;
            Temp = true;
        }
    } else if (Fac == Facing::South) {
        congestion.z++;
        if (getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) != 1 && getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) != 3 &&
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) != 1 &&
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) != 3) {
            getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) = 0;
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) = 0;
            Coord.Z++;
            Temp = true;
        }
    } else {
        congestion.x--;
        if (getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) != 1 && getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) != 3 &&
            getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) != 1 &&
            getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) != 3) {
            getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) = 0;
            getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) = 0;
            Coord.X--;
            Temp = true;
        }
    }
    if (Temp) {
        updateSinglePortUsedSpaceHelper(Coord);
    }
    return {.x = Coord.X, .y = Coord.Y, .z = Coord.Z};
}

void Router::updateSinglePortUsedSpaceHelper(const Coordinate& Coord) {
    if (Coord.X > 0) {
        if (getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) != 3)
            getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) = (getUsedSpace(Coord.X - 1, Coord.Y, Coord.Z) != 1) ? 0 : 1;
        if (getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) != 3)
            getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) =
                (getUsedSpace(Coord.X - 1, Coord.Y - 1, Coord.Z) != 1) ? 0 : 1;
    }
    if (Coord.X < D.Width - 1) {
        if (getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) != 3)
            getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) = (getUsedSpace(Coord.X + 1, Coord.Y, Coord.Z) != 1) ? 0 : 1;
        if (getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) != 3)
            getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) =
                (getUsedSpace(Coord.X + 1, Coord.Y - 1, Coord.Z) != 1) ? 0 : 1;
    }
    if (Coord.Z > 0) {
        if (getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) != 3)
            getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) = (getUsedSpace(Coord.X, Coord.Y, Coord.Z - 1) != 1) ? 0 : 1;
        if (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) != 3)
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) =
                (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z - 1) != 1) ? 0 : 1;
    }
    if (Coord.Z < D.Length - 1) {
        if (getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) != 3)
            getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) = (getUsedSpace(Coord.X, Coord.Y, Coord.Z + 1) != 1) ? 0 : 1;
        if (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) != 3)
            getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) =
                (getUsedSpace(Coord.X, Coord.Y - 1, Coord.Z + 1) != 1) ? 0 : 1;
    }
}

bool Router::routingLastRepeater(
    Router::Point* CongestionP) { // TODO need to check the repeater's direction then move the repeater
    //&& if failed, need to reroute to implement the condition for placing repeater
    Router::Point* tail = CongestionP;
    bool RetFlag = false;
    while (tail && tail->P && tail->P->P && (tail->length >= 3)) {
        if ((tail->ori == tail->P->ori) && (tail->P->P->ori == tail->P->ori) && (tail->Loc.y == tail->P->Loc.y) &&
            (tail->P->P->Loc.y == tail->P->Loc.y)) {
            tail->P->length = MAX_NUM_OF_WIRE + 1;
            RetFlag = true;
            break;
        }
        tail = tail->P;
    }
    if (RetFlag) {
        CongestionP->length = -1;
        routingLastRepeaterHelper(CongestionP);
        DOUT(<< "Repeater Routing Success!" << std::endl);
    } else {
        DOUT(<< "Repeater Routing Failed!" << std::endl);
    }
    return RetFlag;
}

int Router::routingLastRepeaterHelper(Router::Point* RecurP) {
    if (RecurP->P->length == MAX_NUM_OF_WIRE + 1) {
        RecurP->length = 1;
        return 2;
    } else {
        RecurP->length = routingLastRepeaterHelper(RecurP->P);
        return RecurP->length + 1;
    }
    return -1;
}