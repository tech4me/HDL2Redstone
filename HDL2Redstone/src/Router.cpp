#include <cmath>
#include <queue>
#include <set>

#include <Component.hpp>
#include <Connection.hpp>
#include <Router.hpp>

#define MAX_NUM_OF_WIRE 12

using namespace HDL2Redstone;
void Router::InitPortUsedSpaceHelper(Router::coord& Loc,std::tuple<uint16_t, uint16_t, uint16_t>& Space){
    if (Loc.x > 0) {
        UsedSpace[Loc.x - 1][Loc.y][Loc.z] = (UsedSpace[Loc.x - 1][Loc.y][Loc.z] == 1) ? 1 : 2;
    }
    if (Loc.x < std::get<0>(Space) - 1) {
        UsedSpace[Loc.x + 1][Loc.y][Loc.z] = (UsedSpace[Loc.x + 1][Loc.y][Loc.z] == 1) ? 1 : 2;
    }
    if (Loc.y > 0) {
        UsedSpace[Loc.x][Loc.y - 1][Loc.z] = (UsedSpace[Loc.x][Loc.y - 1][Loc.z] == 1) ? 1 : 2;
    }
    if (Loc.z > 0) {
        UsedSpace[Loc.x][Loc.y][Loc.z - 1] = (UsedSpace[Loc.x][Loc.y][Loc.z - 1] == 1) ? 1 : 2;
    }
    if (Loc.z < std::get<2>(Space) - 1) {
        UsedSpace[Loc.x][Loc.y][Loc.z + 1] = (UsedSpace[Loc.x][Loc.y][Loc.z + 1] == 1) ? 1 : 2;
    }    
}
void Router::InitPortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac,std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    Router::coord ret;
    ret.x = std::get<0>(Loc);
    ret.y = std::get<1>(Loc);
    ret.z = std::get<2>(Loc);
    if (Fac == Facing::North) {
        WI[ret.x][ret.y][ret.z - 1].ComponentSpace = 2;
        WI[ret.x][ret.y - 1][ret.z - 1].ComponentSpace = 2;
        ret.z--;
    }
    else if (Fac == Facing::East) {
        WI[ret.x + 1][ret.y][ret.z].ComponentSpace = 2;
        WI[ret.x + 1][ret.y - 1][ret.z].ComponentSpace = 2;
        ret.x++;
    }
    // TODO: Add vertical pin
    else if (Fac == Facing::South) {
        WI[ret.x][ret.y][ret.z + 1].ComponentSpace = 2;
        WI[ret.x][ret.y - 1][ret.z + 1].ComponentSpace = 2;
        ret.z++;
    }
    else{
        WI[ret.x - 1][ret.y][ret.z].ComponentSpace = 2;
        WI[ret.x - 1][ret.y - 1][ret.z].ComponentSpace = 2;
        ret.x--;
    }
    InitPortUsedSpaceHelper(ret,Space);
}
void Router::updateSinglePortUsedSpaceHelper(Router::coord& Loc,std::tuple<uint16_t, uint16_t, uint16_t>& Space){
    if (Loc.x > 0) {
        UsedSpace[Loc.x - 1][Loc.y][Loc.z] = (UsedSpace[Loc.x - 1][Loc.y][Loc.z] != 1) ? 0 : 1;
    }
    if (Loc.x < std::get<0>(Space) - 1) {
        UsedSpace[Loc.x + 1][Loc.y][Loc.z] = (UsedSpace[Loc.x + 1][Loc.y][Loc.z] != 1) ? 0 : 1;
    }
    if (Loc.y > 0) {
        UsedSpace[Loc.x][Loc.y - 1][Loc.z] = (UsedSpace[Loc.x][Loc.y - 1][Loc.z] != 1) ? 0 : 1;
    }
    if (Loc.z > 0) {
        UsedSpace[Loc.x][Loc.y][Loc.z - 1] = (UsedSpace[Loc.x][Loc.y][Loc.z - 1] != 1) ? 0 : 1;
    }
    if (Loc.z < std::get<2>(Space) - 1) {
        UsedSpace[Loc.x][Loc.y][Loc.z + 1] = (UsedSpace[Loc.x][Loc.y][Loc.z + 1] != 1) ? 0 : 1;
    }     
}
Router::coord Router::updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac,
                                                coord& congestion, std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    Router::coord ret;
    ret.x = std::get<0>(Loc);
    ret.y = std::get<1>(Loc);
    ret.z = std::get<2>(Loc);
    congestion.x = std::get<0>(Loc);
    congestion.y = std::get<1>(Loc);
    congestion.z = std::get<2>(Loc);
    bool temp=false;
    if (Fac == Facing::North) {
        congestion.z--;
        if (UsedSpace[ret.x][ret.y][ret.z - 1] != 1) {
            UsedSpace[ret.x][ret.y][ret.z - 1] = 0;
            UsedSpace[ret.x][ret.y - 1][ret.z - 1] = 0;
            // WI[ret.x][ret.y][ret.z - 1].ComponentSpace = 2;
            // WI[ret.x][ret.y - 1][ret.z - 1].ComponentSpace = 2;
            ret.z--;
            temp = true;
        }
    }
    else if (Fac == Facing::East) {
        congestion.x++;
        if (UsedSpace[ret.x + 1][ret.y][ret.z] != 1) {
            UsedSpace[ret.x + 1][ret.y][ret.z] = 0;
            UsedSpace[ret.x + 1][ret.y - 1][ret.z] = 0;
            // WI[ret.x + 1][ret.y][ret.z].ComponentSpace = 2;
            // WI[ret.x + 1][ret.y - 1][ret.z].ComponentSpace = 2;
            ret.x++;
            temp = true;
        }
    }
    else if (Fac == Facing::South) {
        congestion.z++;
        if (UsedSpace[ret.x][ret.y][ret.z + 1] != 1) {
            UsedSpace[ret.x][ret.y][ret.z + 1] = 0;
            UsedSpace[ret.x][ret.y - 1][ret.z + 1] = 0;
            // WI[ret.x][ret.y][ret.z + 1].ComponentSpace = 2;
            // WI[ret.x][ret.y - 1][ret.z + 1].ComponentSpace = 2;
            ret.z++;
            temp = true;
        }
    }
    else (Fac == Facing::West) {
        congestion.x--;
        if (UsedSpace[ret.x - 1][ret.y][ret.z] != 1) {
            UsedSpace[ret.x - 1][ret.y][ret.z] = 0;
            UsedSpace[ret.x - 1][ret.y - 1][ret.z] = 0;
            // WI[ret.x - 1][ret.y][ret.z].ComponentSpace = 2;
            // WI[ret.x - 1][ret.y - 1][ret.z].ComponentSpace = 2;
            ret.x--;
            temp = true;
        }
    }
    if(temp){
        updateSinglePortUsedSpaceHelper(ret,Space);    
    }
    return ret;
}
Router::Router(const Design& D) {
    std::tuple<uint16_t, uint16_t, uint16_t> Space = D.getSpace();
    UsedSpace = new int**[std::get<0>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        UsedSpace[i] = new int*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            UsedSpace[i][j] = new int[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                UsedSpace[i][j][k] = 0;
            }
        }
    }

    // add wire info for each block all over the space
    WI = new WireInfo**[std::get<0>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        WI[i] = new WireInfo*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            WI[i][j] = new WireInfo[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                WI[i][j][k].ComponentSpace = 0;
                WI[i][j][k].C_ptr.clear();
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
    }
    for (auto const& i : UsedComponentSpace) {
        for (uint16_t j = std::get<0>(i.first); j < std::get<0>(i.second); j++) {
            for (uint16_t k = std::get<1>(i.first); k < std::get<1>(i.second); k++) {
                for (uint16_t z = std::get<2>(i.first); z < std::get<2>(i.second); z++) {
                    UsedSpace[j][k][z] = 1;
                    WI[j][k][z].ComponentSpace = 1;
                    if (j > 0) {
                        UsedSpace[j - 1][k][z] = (UsedSpace[j - 1][k][z] == 1) ? 1 : 2;
                        WI[j - 1][k][z].ComponentSpace = 1;
                    }
                    if (j < std::get<0>(Space) - 1) {
                        UsedSpace[j + 1][k][z] = (UsedSpace[j + 1][k][z] == 1) ? 1 : 2;
                        WI[j + 1][k][z].ComponentSpace = 1;
                    }

                    if (k > 0) {
                        UsedSpace[j][k - 1][z] = (UsedSpace[j][k - 1][z] == 1) ? 1 : 2;
                        WI[j][k - 1][z].ComponentSpace = 1;
                    }
                    if (k < std::get<1>(Space) - 1) {
                        UsedSpace[j][k + 1][z] =
                            (UsedSpace[j][k + 1][z] == 1) ? 1 : 2; // TODO veritcal pin maybe possible
                        WI[j][k + 1][z].ComponentSpace = 1;
                    }

                    if (z > 0) {
                        UsedSpace[j][k][z - 1] = (UsedSpace[j][k][z - 1] == 1) ? 1 : 2;
                        WI[j][k][z - 1].ComponentSpace = 1;
                    }
                    if (z < std::get<2>(Space) - 1) {
                        UsedSpace[j][k][z + 1] = (UsedSpace[j][k][z + 1] == 1) ? 1 : 2;
                        WI[j][k][z + 1].ComponentSpace = 1;
                    }
                }
            }
        }
    }
    for (auto const& j : Connections_) {
        auto SourcePortConnection_ = j->getSourcePortConnection();
        InitPortUsedSpace(((SourcePortConnection_).first)->getPinLocation((SourcePortConnection_).second),
                          ((SourcePortConnection_).first)->getPinFacing((SourcePortConnection_).second),Space);
        auto SinkPortConnection_ = j->getSinkPortConnections();
        for (auto const& k : SinkPortConnection_) {
            InitPortUsedSpace((k.first)->getPinLocation(k.second), (k.first)->getPinFacing(k.second),Space);
        }
    }
    FailedWire_SingleRouting = NULL;
}
void Router::route(Design& D) {
    // auto& Components_ = D.getModuleNetlist().getComponents();
    auto& Connections_ = D.getModuleNetlist().getConnections();
    std::tuple<uint16_t, uint16_t, uint16_t> Space = D.getSpace();
    Router::Point*** P_ =
        new Router::Point**[std::get<0>(Space)]; //[std::get<0>(Space)][std::get<1>(Space)][std::get<2>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        P_[i] = new Router::Point*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            P_[i][j] = new Router::Point[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                P_[i][j][k].Loc.x = i;
                P_[i][j][k].Loc.y = j;
                P_[i][j][k].Loc.z = k;
            }
        }
    }
    // std::cout<<"\nINIT FOUND wire status: "<<UsedSpace[21][0][10]<<std::endl;
    //  if(WI[33][12][9].C_ptr)
    //  std::cout<<"\nINIT wire name: "<<WI[21][0][10].C_ptr->getName()<<" "<<UsedSpace[33][12][9]<<std::endl;
    /*TODO
    check unablerouting is 0 or 1 or 2 or 3
    to reroute everything,
    1. reconstruct router
    2. keep all wires results, setRouted(0)
    3. after routing this wire, check every wire can keep same result or reroute
    */
    for (int i = 0; i < Connections_.size(); i++) {
        bool all_routed = false;
        FailedWire_SingleRouting = NULL;
        for (auto& it : Connections_) {
            if (it->Result.empty()) {
                // reset P_ for routing
                for (int i = 0; i < std::get<0>(Space); i++) {
                    for (int j = 0; j < std::get<1>(Space); j++) {
                        for (int k = 0; k < std::get<2>(Space); k++) {
                            P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                            P_[i][j][k].length = -1;
                            P_[i][j][k].ori = HDL2Redstone::Orientation::ZeroCW;
                            P_[i][j][k].P = NULL;
                            P_[i][j][k].visited = 0;
                            P_[i][j][k].inserted = 0;
                        }
                    }
                }
                std::cout << "Routing: " << it->getName() << std::endl;
                if (!RegularRoute(D, *it, Space, P_)) {
                    std::cout << "Routing: " << it->getName() << " failed" << std::endl;
                    ;
                }
                //  std::cout<<"\nFOUND wire status: "<<UsedSpace[9][0][24]<<"
                //  "<<WI[9][0][24].ComponentSpace<<std::endl; if(!WI[9][0][24].C_ptr.empty()){
                //      for (auto it: WI[9][0][24].C_ptr)
                //     std::cout<<"\nFOUND wire name: "<<it->getName()<<std::endl;
                //  }else{
                //      std::cout<<"\nFOUND wire EMPTY"<<std::endl;
                //  }
                if (FailedWire_SingleRouting) {
                    break;
                }
            }
        }
        if (FailedWire_SingleRouting == NULL) {
            std::cout << "FUll Routing Success!!!" << std::endl;
            break;
        } else if (FailedWire_SingleRouting->getUnableRouting() == 1) {
            Deconstructor(Space);
            Reconstructor(D);
            FailedWire_SingleRouting->setUnableRouting(2);
            FailedWire_SingleRouting->Result.clear();
            std::cout << "Try Routing: " << FailedWire_SingleRouting->getName() << " first" << std::endl;
            // reset P_ for routing
            for (int i = 0; i < std::get<0>(Space); i++) {
                for (int j = 0; j < std::get<1>(Space); j++) {
                    for (int k = 0; k < std::get<2>(Space); k++) {
                        P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                        P_[i][j][k].length = -1;
                        P_[i][j][k].ori = HDL2Redstone::Orientation::ZeroCW;
                        P_[i][j][k].P = NULL;
                        P_[i][j][k].visited = 0;
                        P_[i][j][k].inserted = 0;
                    }
                }
            }
            auto FailedWire_SingleRouting_temp = FailedWire_SingleRouting;
            FailedWire_SingleRouting = NULL;
            if (!RegularRoute(D, *FailedWire_SingleRouting_temp, Space, P_)) {
                std::cout << "Routing: " << FailedWire_SingleRouting_temp->getName() << " failed" << std::endl;
                ;
            } else {
                for (auto& it : Connections_) {
                    if (FailedWire_SingleRouting_temp->getName() != it->getName()) {
                        CheckandKeepResult(*it, Space);
                    }
                }
            }
        } else if (FailedWire_SingleRouting->getUnableRouting() == 3) {
            std::cout << FailedWire_SingleRouting->getName() << " is unable to route second time, Routing failed"
                      << std::endl;
            break;
        }
    }
    std::cout << "Routing Done" << std::endl;
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            delete[] P_[i][j];
            delete[] WI[i][j];
        }
        delete[] P_[i];
        delete[] WI[i];
    }
    delete[] P_;
    delete[] WI;
    // debugging print
    /*
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                if (i == 5)
                    std::cout << UsedSpace[i][j][k] << " ";
            }
            if (i == 5)
            std::cout << std::endl;
        }
    }
    */
}
bool Router::HelperCheckUpdateGraph(Router::Point* Parent, Router::Point* Current) {
    bool RetFlag = true;
    auto ori_ = Current->ori;
    if (Parent->Loc.x > Current->Loc.x) {
        ori_ = HDL2Redstone::Orientation::OneCW;
    } else if (Parent->Loc.x < Current->Loc.x) {
        ori_ = HDL2Redstone::Orientation::ThreeCW;
    } else if (Parent->Loc.z > Current->Loc.z) {
        ori_ = HDL2Redstone::Orientation::TwoCW;
    } else if (Parent->Loc.z < Current->Loc.z) {
        ori_ = HDL2Redstone::Orientation::ZeroCW;
    }
    if (Parent->length == MAX_NUM_OF_WIRE + 1) {
        if (Parent->ori != ori_) {
            RetFlag = false;
        } else if (Parent->Loc.y != Current->Loc.y) {
            RetFlag = false;
        } else {
            if (Current->cost == Parent->cost + 1) {
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
            if (Current->cost == Parent->cost + 1) {
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
        if (Current->cost == Parent->cost + 1) {
            if ((Parent->Loc.y == Current->Loc.y) && (Current->P->Loc.y != Current->Loc.y)) {
                Current->length = Parent->length + 1;
            } else if ((Parent->Loc.y == Current->Loc.y) && (Current->P->Loc.y == Current->Loc.y)) {
                //1 5 9 13; 3, 7, 11,15
                //if(Current->Loc.y % 4 == 3){
                    if (ori_ == HDL2Redstone::Orientation::ZeroCW || ori_ == HDL2Redstone::Orientation::TwoCW) {
                        Current->length = Parent->length + 1;
                    } else {
                        if (Current->ori == HDL2Redstone::Orientation::ZeroCW || Current->ori == HDL2Redstone::Orientation::TwoCW) {
                            RetFlag = false;
                        } else {
                            Current->length = Parent->length + 1;
                        }
                    }
                // }else{
                //     if (ori_ == HDL2Redstone::Orientation::OneCW || ori_ == HDL2Redstone::Orientation::ThreeCW) {
                //         Current->length = Parent->length + 1;
                //     } else {
                //         if (Current->ori == HDL2Redstone::Orientation::OneCW || Current->ori == HDL2Redstone::Orientation::ThreeCW) {
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
void Router::checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z, Router::Point***& P_,
                              std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                              Router::Point*& TempP, std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    if ((!UsedSpace[x][y][z]) && (!UsedSpace[x][y - 1][z])) {
        // check the node is in for its potential path
        // check up to +- 2 unit are clean
        if ((P_[x][y][z].cost >= TempP->cost + 1)) {
            bool legal = 1;
            // this while loop is checking self wire illegal situation
            Router::Point* temp_ptr = TempP->P;
            while (temp_ptr) {
                if ((temp_ptr->Loc.y < std::get<1>(Space) - 2) &&
                    ((y == temp_ptr->Loc.y + 1) || (y == temp_ptr->Loc.y + 2)) && (temp_ptr->Loc.x == x) &&
                    (temp_ptr->Loc.z == z)) {
                    legal = 0;
                    break;
                }
                if ((temp_ptr->Loc.y > 0) && (y == temp_ptr->Loc.y - 1) && (temp_ptr->Loc.x == x) &&
                    (temp_ptr->Loc.z == z)) {
                    legal = 0;
                    break;
                }
                temp_ptr = temp_ptr->P;
            }
            if (legal) {
                if (HelperCheckUpdateGraph(TempP, &P_[x][y][z])) {
                    P_[x][y][z].cost = TempP->cost + 1;
                    P_[x][y][z].P = TempP;
                    if (!P_[x][y][z].visited && !P_[x][y][z].inserted) {
                        P_[x][y][z].inserted = 1;
                        Q.push(&P_[x][y][z]);
                    }
                }
            }
        }
    }
}
int Router::HelperRoutingLastRepeater(Router::Point* RecurP) {
    if (RecurP->P->length == MAX_NUM_OF_WIRE + 1) {
        RecurP->length = 1;
        return 2;
    } else {
        RecurP->length = HelperRoutingLastRepeater(RecurP->P);
        return RecurP->length + 1;
    }
    return -1;
}
bool Router::RoutingLastRepeater(Router::Point* CongestionP) {
    Router::Point* tail = CongestionP;
    bool RetFlag = false;
    while (tail && tail->P && tail->P->P && (tail->length >= 3)) {
        if ((tail->ori == tail->P->ori) && (tail->P->P->ori == tail->P->ori)) {
            tail->P->length = MAX_NUM_OF_WIRE + 1;
            RetFlag = true;
            break;
        }
        tail = tail->P;
    }
    if (RetFlag) {
        CongestionP->length = -1;
        HelperRoutingLastRepeater(CongestionP);
        std::cout << "Repeater Routing Success!" << std::endl;
    } else {
        std::cout << "Repeater Routing Failed!" << std::endl;
    }
    return RetFlag;
}
bool Router::RegularRoute(Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space,
                          Router::Point***& P_) {
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
        std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z
                  << " because of other wires routing" << std::endl;
        std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                  << std::endl;
        if (!ReRouteStartRouting(congestionP, Space, P_, D)) {
            return false;
        }
        // call update again TODO not good
        start = Router::updateSinglePortUsedSpace(startPin, SrcFacing, congestionP);
    }
    // int i = 0;
    for (auto it = PortConnection_.begin(); it != PortConnection_.end(); ++it) {
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
            if (!ReRouteStartRouting(congestionP, Space, P_, D)) {
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
    P_[start.x][start.y][start.z].cost = 0;
    P_[start.x][start.y][start.z].length = 1;
    P_[start.x][start.y][start.z].inserted = 1;
    Q.push(&P_[start.x][start.y][start.z]);
    bool done = 0;
    //  std::cout<<"\n2222FOUND wire status: "<<UsedSpace[9][0][24]<<"   "<<WI[9][0][24].ComponentSpace<<std::endl;
    //  if(!WI[9][0][24].C_ptr.empty()){
    //      for (auto it: WI[9][0][24].C_ptr)
    //     std::cout<<"\n2222FOUND wire name: "<<it->getName()<<std::endl;
    //  }else{
    //      std::cout<<"\n2222FOUND wire EMPTY"<<std::endl;
    //  }
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
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x < std::get<0>(Space) - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z < std::get<2>(Space) - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }

        if (TempP->Loc.x && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x && (TempP->Loc.y < std::get<1>(Space) - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y > 1)/* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y < std::get<1>(Space) - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y > 1)/* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
    }
    C.setRouted(0);
    Point* ptr = NULL;
    for (auto it : end) {
        if (P_[it.x][it.y][it.z].P) {
            C.setRouted(1);
            uint16_t TempX, TempY, TempZ;
            TempX = it.x;
            TempY = it.y;
            TempZ = it.z;
            if (P_[it.x][it.y][it.z].length >= MAX_NUM_OF_WIRE + 1) {
                std::cout << "Routing failed: Repeater is placed at the endpin " << TempX << " " << TempY << " "
                          << TempZ << std::endl;
                std::cout << "Try Routing the repeater ..." << std::endl;
                RoutingLastRepeater(&P_[it.x][it.y][it.z]);
            }
            C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                     D.CellLib.getCellPtr("WIRE"), P_[it.x][it.y][it.z].ori));
            // WI[TempX][TempY][TempZ].C_ptr = &C;
            ptr = P_[it.x][it.y][it.z].P;
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
                                                             D.CellLib.getCellPtr("BUF"), ptr->ori));
                } else {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("WIRE"), ptr->ori));
                }
                // WI[TempX][TempY][TempZ].C_ptr = &C;
                ptr = ptr->P;
            }
        } else {
            std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z << " to " << it.x << ", "
                      << it.y << ", " << it.z << std::endl;
            FailedWire_SingleRouting = &C;
            retFlag = false;
            if (C.getUnableRouting() == 0) {
                C.setUnableRouting(1);
            } else if (C.getUnableRouting() == 2) {
                C.setUnableRouting(3);
            }
        }
    }

    auto IllegalPoints = C.checkRouteResult();
    if (!IllegalPoints.empty()) {
        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> congestionPoints_prev;
        return ReRouteIllegal(C, IllegalPoints, Space, P_, D, congestionPoints_prev);
    }
    updateUsedSpace(C, Space);
    for (auto itt = C.Result.begin(); itt != C.Result.end(); ++itt) {
        // if(C.getName()=="dout"){
        //     std::cout << "start point is " << start.x << ", " << start.y << ", " << start.z << std::endl;
        if (std::get<0>(itt->coord) == 9 && std::get<1>(itt->coord) == 0 && std::get<2>(itt->coord) == 24)
            std::cout << "routing" << std::get<0>(itt->coord) << ", " << std::get<1>(itt->coord) << ", "
                      << std::get<2>(itt->coord);
        // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::ZeroCW)
        // std::cout<<"ORI:"<<0<<std::endl;
        // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::OneCW)
        // std::cout<<"ORI:"<<1<<std::endl;
        // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::TwoCW)
        // std::cout<<"ORI:"<<2<<std::endl;
        // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].ori==HDL2Redstone::Orientation::ThreeCW)
        // std::cout<<"ORI:"<<3<<std::endl;
        // if(P_[std::get<0>(itt->coord)][std::get<1>(itt->coord)][std::get<2>(itt->coord)].length >= MAX_NUM_OF_WIRE +
        // 1){
        //    std::cout<<"HIT!!!"<<3<<std::endl;
        // }
        // }
    }
    return retFlag;
}

void Router::updateUsedSpace(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    std::set<Connection::ConnectionResult, Connection::resultcomp>& Result = C.Result;
    for (const auto& entry : Result) {
        Router::coord entry_temp;
        entry_temp.x = std::get<0>(entry.coord);
        entry_temp.y = std::get<1>(entry.coord);
        entry_temp.z = std::get<2>(entry.coord);
        UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z] = 1;
        WI[entry_temp.x][entry_temp.y + 1][entry_temp.z].C_ptr.insert(&C);
        UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z] = 1;
        WI[entry_temp.x][entry_temp.y][entry_temp.z].C_ptr.insert(&C);
        if ((entry_temp.y > 0)) {
            UsedSpace[entry_temp.x][entry_temp.y - 1][entry_temp.z] = 1;
            WI[entry_temp.x][entry_temp.y - 1][entry_temp.z].C_ptr.insert(&C);
        }
        if ((entry_temp.y < std::get<1>(Space) - 2)) {
            UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z] = 1;
            WI[entry_temp.x][entry_temp.y + 2][entry_temp.z].C_ptr.insert(&C);
        }
        if (entry_temp.x > 0) {
            UsedSpace[entry_temp.x - 1][entry_temp.y + 1][entry_temp.z] = 1;
            WI[entry_temp.x - 1][entry_temp.y + 1][entry_temp.z].C_ptr.insert(&C);
            UsedSpace[entry_temp.x - 1][entry_temp.y][entry_temp.z] = 1;
            WI[entry_temp.x - 1][entry_temp.y][entry_temp.z].C_ptr.insert(&C);
            if ((entry_temp.y < std::get<1>(Space) - 2)) {
                UsedSpace[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z] = 1;
                WI[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z].C_ptr.insert(&C);
            }
        }
        if (entry_temp.x < std::get<0>(Space) - 1) {
            UsedSpace[entry_temp.x + 1][entry_temp.y + 1][entry_temp.z] = 1;
            WI[entry_temp.x + 1][entry_temp.y + 1][entry_temp.z].C_ptr.insert(&C);
            UsedSpace[entry_temp.x + 1][entry_temp.y][entry_temp.z] = 1;
            WI[entry_temp.x + 1][entry_temp.y][entry_temp.z].C_ptr.insert(&C);
            if ((entry_temp.y < std::get<1>(Space) - 2)) {
                UsedSpace[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z] = 1;
                WI[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z].C_ptr.insert(&C);
            }
        }
        if (entry_temp.z > 0) {
            UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z - 1] = 1;
            WI[entry_temp.x][entry_temp.y + 1][entry_temp.z - 1].C_ptr.insert(&C);
            UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z - 1] = 1;
            WI[entry_temp.x][entry_temp.y][entry_temp.z - 1].C_ptr.insert(&C);
            if ((entry_temp.y < std::get<1>(Space) - 2)) {
                UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1] = 1;
                WI[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1].C_ptr.insert(&C);
            }
        }
        if (entry_temp.z < std::get<2>(Space) - 1) {
            UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z + 1] = 1;
            WI[entry_temp.x][entry_temp.y + 1][entry_temp.z + 1].C_ptr.insert(&C);
            UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z + 1] = 1;
            WI[entry_temp.x][entry_temp.y][entry_temp.z + 1].C_ptr.insert(&C);
            if ((entry_temp.y < std::get<1>(Space) - 2)) {
                UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1] = 1;
                WI[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1].C_ptr.insert(&C);
            }
        }
    }
}
bool Router::HelperReRouteIllegal(Connection& C,
                                  std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& local_congestion_points,
                                  std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_, Design& D) {
    std::string rollback_wire_name = C.getName();
    bool break_loop = false;

    do {
        for (auto it = local_congestion_points.begin(); it != local_congestion_points.end(); ++it) {
            // std::cout<<"to set to 3: "<<std::get<0>(*it)<<" "<<std::get<1>(*it)<<" "<<std::get<2>(*it)<<" used
            // "<<UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)]<<std::endl;
            if (UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)] != 1) {
                UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)] = 3;
            }
        }
        // route congested wire again
        for (int i = 0; i < std::get<0>(Space); i++) {
            for (int j = 0; j < std::get<1>(Space); j++) {
                for (int k = 0; k < std::get<2>(Space); k++) {
                    P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                    P_[i][j][k].length = -1;
                    P_[i][j][k].ori = HDL2Redstone::Orientation::ZeroCW;
                    P_[i][j][k].P = NULL;
                    P_[i][j][k].visited = 0;
                    P_[i][j][k].inserted = 0;
                }
            }
        }
        C.Result.clear();
        std::cout << "ReRouting for Illegal: " << rollback_wire_name << std::endl;
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>> RetIllegalPoints;
        bool ReRouteFlag =
            HelperReRouteforIllegalRegularRoute(D, C, Space, P_, RetIllegalPoints, local_congestion_points);
        for (auto it = local_congestion_points.begin(); it != local_congestion_points.end(); ++it) {
            if (UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)] == 3) {
                UsedSpace[std::get<0>(*it)][std::get<1>(*it)][std::get<2>(*it)] = 0;
            }
        }
        if (ReRouteFlag) {
            std::cout << "ReRouting for Illegal Success: " << rollback_wire_name << std::endl;
            return true;
        } else {
            if (RetIllegalPoints.empty()) {
                std::cout << "ReRouting for Illegal Failed: " << rollback_wire_name << std::endl;
                return false;
            }
            bool temp_reroute_flag = false;
            std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> local_congestion_points_temp;
            for (auto nIterator = local_congestion_points.begin(); nIterator != local_congestion_points.end();
                 nIterator++) {
                bool already_inserted = false;
                for (auto mIterator = RetIllegalPoints.begin(); mIterator != RetIllegalPoints.end(); mIterator++) {
                    if ((std::get<0>(*nIterator) == std::get<0>(*mIterator)) &&
                        (std::get<1>(*nIterator) == std::get<1>(*mIterator)) &&
                        (std::get<2>(*nIterator) == std::get<2>(*mIterator))) {
                        if (std::get<3>(*nIterator) == 1) {
                            std::cout << "ReRouting for Illegal Failed: " << rollback_wire_name << std::endl;
                            return false;
                        }
                        // std::get<3>(*nIterator) = 1;
                        // std::get<1>(*nIterator)-= (std::get<3>(*nIterator)+1);
                        local_congestion_points_temp.insert(std::make_tuple(
                            std::get<0>(*nIterator), std::get<1>(*nIterator) - (std::get<3>(*nIterator) + 1),
                            std::get<2>(*nIterator), 1));
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
            std::cout << "ReRouting for Illegal Failed: " << rollback_wire_name << ", Still Trying ..." << std::endl;
            break_loop = !temp_reroute_flag;
        }
    } while (!break_loop);
    return false;
}
bool Router::ReRouteIllegal(Connection& C,
                            std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
                            std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_, Design& D,
                            std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints_prev) {
    std::string rollback_wire_name = C.getName();
    std::cout << "Wire " << rollback_wire_name << " routing illegal: " << std::endl;
    // for (auto& test_t: C.Result){
    //     if(std::get<0>(test_t.coord) == 15)
    //     std::cout<<"point list "<<std::get<0>(test_t.coord)<<" "<<std::get<1>(test_t.coord)<<"
    //     "<<std::get<2>(test_t.coord)<<std::endl;
    // }
    //TODO:can be optimaze its code
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> local_congestion_points;
    for(auto& i: congestionPoints){
        std::cout << "    " << std::get<0>(i) << ", " << std::get<1>(i) << ", " << std::get<2>(i)
                  << " Type: " << std::get<3>(i) << std::endl;
        bool temp_flag = false;
        for(auto& it: congestionPoints_prev){
            if( (std::get<0>(it) == std::get<0>(i)) && (std::get<1>(it) == std::get<1>(i)) && (std::get<2>(it) == std::get<2>(i))){
                if(std::get<3>(it)==1){
                    std::cout << "        get it 3 times: rerouting for illegal failed"<< std::endl;
                    return false; 
                }
            }
            if( (std::get<0>(it) == std::get<0>(i)) && (std::get<1>(it) == std::get<1>(i)+std::get<3>(i)+1) && (std::get<2>(it) == std::get<2>(i))){
                if(std::get<3>(it)==0){
                    temp_flag = true;
                    std::cout << "        get it twice: disable the lower point for illegal"<< std::endl;
                    congestionPoints_prev.erase(it);
                    congestionPoints_prev.insert(std::make_tuple(std::get<0>(it),std::get<1>(i), std::get<2>(i), 1));
                    break;
                }
            }
        }
        if(!temp_flag){
            local_congestion_points.insert(std::make_tuple(std::get<0>(i), std::get<1>(i) + std::get<3>(i) + 1, std::get<2>(i), 0));
        }
    }
    for (auto it : congestionPoints_prev) {
        std::cout << "    prev " << std::get<0>(it) << ", " << std::get<1>(it) << ", " << std::get<2>(it)<< " converted_type: " << std::get<3>(it) << std::endl;
        local_congestion_points.insert(it);
    }
    // for(auto it = local_congestion_points.begin(); it!=local_congestion_points.end(); ++it){
    //     std::cout <<"    "<<std::get<0>(*it)<<", "<<std::get<1>(*it)<<", "<<std::get<2>(*it)<<" ---
    //     "<<std::get<3>(*it)<<std::endl;
    // }
    return HelperReRouteIllegal(C, local_congestion_points, Space, P_, D);
}

bool Router::ReRouteStartRouting(coord congestionPoint, std::tuple<uint16_t, uint16_t, uint16_t>& Space,
                                 Router::Point***& P_, Design& D) {
    auto C_ptr = WI[congestionPoint.x][congestionPoint.y][congestionPoint.z].C_ptr;
    if (C_ptr.empty()) {
        std::cout << "ReRoute Failed: congest each other" << std::endl;
        return false;
    }
    std::set<std::string> rollback_wire_name;
    // roll back congestion wires
    for (auto it : C_ptr) {
        rollback_wire_name.insert(it->getName());
        std::cout << it->getName() << " is rolling back for ReRouting" << std::endl;
        it->Result.clear();
    }
    // roll back congestion wires
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                if ((!WI[i][j][k].C_ptr.empty())) {
                    std::set<Connection*> temp = WI[i][j][k].C_ptr;
                    for (auto z : WI[i][j][k].C_ptr) {
                        if (rollback_wire_name.find(z->getName()) != rollback_wire_name.end()) {
                            temp.erase(z);
                            // WI[i][j][k].C_ptr->Result.clear();
                        }
                    }
                    WI[i][j][k].C_ptr = temp;
                    if (WI[i][j][k].ComponentSpace != 1 && WI[i][j][k].C_ptr.empty()) {
                        UsedSpace[i][j][k] = 0;
                    }
                }
            }
        }
    }
    // force congestion point is occupied
    UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z] = 3;
    if (congestionPoint.y < std::get<1>(Space) - 1) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] == 0) {
            UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.y > 0) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] == 0) {
            UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.x > 0) {
        if (UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] == 0) {
            UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.x < std::get<0>(Space) - 1) {
        if (UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] == 0) {
            UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.z > 0) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] == 0) {
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] = 3;
        }
    }
    if (congestionPoint.z < std::get<2>(Space) - 1) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] == 0) {
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] = 3;
        }
    }
    bool ReRouteFlag = true;
    for (auto it : C_ptr) {
        // route congested wire again
        for (int i = 0; i < std::get<0>(Space); i++) {
            for (int j = 0; j < std::get<1>(Space); j++) {
                for (int k = 0; k < std::get<2>(Space); k++) {
                    P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                    P_[i][j][k].length = -1;
                    P_[i][j][k].ori = HDL2Redstone::Orientation::ZeroCW;
                    P_[i][j][k].P = NULL;
                    P_[i][j][k].visited = 0;
                    P_[i][j][k].inserted = 0;
                }
            }
        }
        std::cout << "ReRouting: " << it->getName() << std::endl;
        if (!RegularRoute(D, *it, Space, P_)) {
            std::cout << "ReRouting Failed: " << it->getName() << std::endl;
            ReRouteFlag = false;
        } else {
            std::cout << "ReRouting Success: " << it->getName() << std::endl;
        }
    }
    if (UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z] == 3) {
        UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z] = 0;
    }
    if (congestionPoint.y < std::get<1>(Space) - 1) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] == 3) {
            UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.y > 0) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] == 3) {
            UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.x > 0) {
        if (UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] == 3) {
            UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.x < std::get<0>(Space) - 1) {
        if (UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] == 3) {
            UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.z > 0) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] == 3) {
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] = 0;
        }
    }
    if (congestionPoint.z < std::get<2>(Space) - 1) {
        if (UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] == 3) {
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] = 0;
        }
    }
    // roll back P_
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                P_[i][j][k].cost = std::get<0>(Space) * std::get<1>(Space) * std::get<2>(Space);
                P_[i][j][k].length = -1;
                P_[i][j][k].ori = HDL2Redstone::Orientation::ZeroCW;
                P_[i][j][k].P = NULL;
                P_[i][j][k].visited = 0;
                P_[i][j][k].inserted = 0;
            }
        }
    }
    return ReRouteFlag;
}

bool Router::HelperReRouteforIllegalRegularRoute(
    Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& RetIllegalPoints,
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints) {
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
        std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z
                  << " because of other wires routing" << std::endl;
        std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                  << std::endl;
        if (UsedSpace[congestionP.x][congestionP.y][congestionP.z] ==
            3) { // Here to check the congestion point is set as non-reachable, it should fail
            RetIllegalPoints.insert(startPin);
            return false;
        } else {
            if (!ReRouteStartRouting(congestionP, Space, P_, D)) {
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
            std::cout << "FAIL routing to " << temp_result.x << ", " << temp_result.y << ", " << temp_result.z
                      << " because of other wires routing" << std::endl;
            std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z
                  << std::endl;
            if (UsedSpace[temp_result.x][temp_result.y][temp_result.z] ==
                3) { // Here to check the congestion point is set as non-reachable, it should fail
                RetIllegalPoints.insert(temp);
                RetforCongestionFlag = true;
            }
            if (!ReRouteStartRouting(congestionP, Space, P_, D)) {
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
    if (RetforCongestionFlag) {
        return false;
    }

    std::vector<Router::coord> endTemp = end;
    std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare> Q;
    P_[start.x][start.y][start.z].cost = 0;
    P_[start.x][start.y][start.z].length = 1;
    P_[start.x][start.y][start.z].inserted = 1;
    Q.push(&P_[start.x][start.y][start.z]);
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
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x < std::get<0>(Space) - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z /*&& (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z < std::get<2>(Space) - 1 /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/ &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }

        if (TempP->Loc.x && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x && (TempP->Loc.y < std::get<1>(Space) - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y > 1) /*&& (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y > 1)/* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y < std::get<1>(Space) - 1) /*&&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y > 1)/* && (TempP->length <= MAX_NUM_OF_WIRE)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1)/* &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)*/) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
    }
    C.setRouted(0);
    Point* ptr = NULL;
    for (auto it : end) {
        if (P_[it.x][it.y][it.z].P) {
            C.setRouted(1);
            uint16_t TempX, TempY, TempZ;
            TempX = it.x;
            TempY = it.y;
            TempZ = it.z;
            if (P_[it.x][it.y][it.z].length >= MAX_NUM_OF_WIRE + 1) {
                std::cout << "Routing failed: Repeater is placed at the endpin " << TempX << " " << TempY << " "
                          << TempZ << std::endl;
                std::cout << "Try Routing the repeater ..." << std::endl;
                RoutingLastRepeater(&P_[it.x][it.y][it.z]);
            }
            C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                     D.CellLib.getCellPtr("WIRE"), P_[it.x][it.y][it.z].ori));
            // WI[TempX][TempY][TempZ].C_ptr = &C;
            ptr = P_[it.x][it.y][it.z].P;
            while (ptr != NULL) {
                uint16_t TempX, TempY, TempZ;
                TempX = ptr->Loc.x;
                TempY = ptr->Loc.y;
                TempZ = ptr->Loc.z;
                if (ptr->length >= MAX_NUM_OF_WIRE + 1) {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("BUF"), ptr->ori));
                } else {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("WIRE"), ptr->ori));
                }
            next:;
                // WI[TempX][TempY][TempZ].C_ptr = &C;
                ptr = ptr->P;
            }
        } else {
            std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z << " to " << it.x << ", "
                      << it.y << ", " << it.z << std::endl;
            FailedWire_SingleRouting = &C;
            retFlag = false;
            if (C.getUnableRouting() == 0) {
                C.setUnableRouting(1);
            } else if (C.getUnableRouting() == 2) {
                C.setUnableRouting(3);
            }
        }
    }
    auto IllegalPoints = C.checkRouteResult();
    if (!IllegalPoints.empty()) {
        return ReRouteIllegal(C, IllegalPoints, Space, P_, D, congestionPoints);
    }
    updateUsedSpace(C, Space);
    // for(auto itt = C.Result.begin(); itt!=C.Result.end(); ++itt)
    // {
    //     if(C.getName()=="x")
    //     std::cout << "routing"
    //                     <<std::get<0>(itt->coord)<<", "<<std::get<1>(itt->coord)<<",
    //                     "<<std::get<2>(itt->coord)<<std::endl;
    // }
    return retFlag;
}
void Router::Deconstructor(std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            delete[] WI[i][j];
        }
        delete[] WI[i];
    }
    delete[] WI;
}
void Router::Reconstructor(const Design& D) {
    std::tuple<uint16_t, uint16_t, uint16_t> Space = D.getSpace();
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                UsedSpace[i][j][k] = 0;
            }
        }
    }

    // add wire info for each block all over the space
    WI = new WireInfo**[std::get<0>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        WI[i] = new WireInfo*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            WI[i][j] = new WireInfo[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                WI[i][j][k].ComponentSpace = 0;
                WI[i][j][k].C_ptr.clear();
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
    }

    for (auto const& i : UsedComponentSpace) {
        for (uint16_t j = std::get<0>(i.first); j < std::get<0>(i.second); j++) {
            for (uint16_t k = std::get<1>(i.first); k < std::get<1>(i.second); k++) {
                for (uint16_t z = std::get<2>(i.first); z < std::get<2>(i.second); z++) {
                    UsedSpace[j][k][z] = 1;
                    WI[j][k][z].ComponentSpace = 1;
                    if (j > 0) {
                        UsedSpace[j - 1][k][z] = (UsedSpace[j - 1][k][z] == 1) ? 1 : 2;
                        WI[j - 1][k][z].ComponentSpace = 1;
                    }
                    if (j < std::get<0>(Space) - 1) {
                        UsedSpace[j + 1][k][z] = (UsedSpace[j + 1][k][z] == 1) ? 1 : 2;
                        WI[j + 1][k][z].ComponentSpace = 1;
                    }

                    if (k > 0) {
                        UsedSpace[j][k - 1][z] = (UsedSpace[j][k - 1][z] == 1) ? 1 : 2;
                        WI[j][k - 1][z].ComponentSpace = 1;
                    }
                    if (k < std::get<1>(Space) - 1) {
                        UsedSpace[j][k + 1][z] =
                            (UsedSpace[j][k + 1][z] == 1) ? 1 : 2; // TODO veritcal pin maybe possible
                        WI[j][k + 1][z].ComponentSpace = 1;
                    }

                    if (z > 0) {
                        UsedSpace[j][k][z - 1] = (UsedSpace[j][k][z - 1] == 1) ? 1 : 2;
                        WI[j][k][z - 1].ComponentSpace = 1;
                    }
                    if (z < std::get<2>(Space) - 1) {
                        UsedSpace[j][k][z + 1] = (UsedSpace[j][k][z + 1] == 1) ? 1 : 2;
                        WI[j][k][z + 1].ComponentSpace = 1;
                    }
                }
            }
        }
    }
    for (auto const& j : Connections_) {
        auto SourcePortConnection_ = j->getSourcePortConnection();
        InitPortUsedSpace(((SourcePortConnection_).first)->getPinLocation((SourcePortConnection_).second),
                          ((SourcePortConnection_).first)->getPinFacing((SourcePortConnection_).second),Space);
        auto SinkPortConnection_ = j->getSinkPortConnections();
        for (auto const& k : SinkPortConnection_) {
            InitPortUsedSpace((k.first)->getPinLocation(k.second), (k.first)->getPinFacing(k.second),Space);
        }
    }
    for (auto const& j : Connections_) {
        if ((j->getUnableRouting() == 2) && (!j->Result.empty())) {
            std::cout << j->getName() << " already restart routing" << std::endl;
            updateUsedSpace(*j, Space);
        }
    }
}
bool Router::CheckandKeepResult(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    bool RetFlag = true;
    // if(C.getName()=="$abc$73$new_n14_"){
    //              std::cout<<"\nstart1FOUND wire status: "<<UsedSpace[24][13][9]<<"
    //              "<<WI[24][13][9].ComponentSpace<<std::endl; if(!WI[24][13][9].C_ptr.empty()){
    //                  for (auto it: WI[24][13][9].C_ptr)
    //                 std::cout<<"\nstart1FOUND wire name: "<<it->getName()<<std::endl;
    //              }else{
    //                  std::cout<<"\nstart1FOUND wire EMPTY"<<std::endl;
    //              }
    //         if (C.Result.empty()) {std::cout<<"\nstart1FOUND result EMPTY"<<std::endl;}
    // }
    if (!C.Result.empty()) {
        for (const auto& R : C.Result) {
            if (UsedSpace[std::get<0>(R.coord)][std::get<1>(R.coord)][std::get<2>(R.coord)] == 1 ||
                UsedSpace[std::get<0>(R.coord)][std::get<1>(R.coord) + 1][std::get<2>(R.coord)] == 1) {
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
        for (int i = 0; i < std::get<0>(Space); i++) {
            for (int j = 0; j < std::get<1>(Space); j++) {
                for (int k = 0; k < std::get<2>(Space); k++) {
                    if ((!WI[i][j][k].C_ptr.empty())) {
                        std::set<Connection*> temp = WI[i][j][k].C_ptr;
                        for (auto z : WI[i][j][k].C_ptr) {
                            if (z->getName() == C.getName()) {
                                temp.erase(z);
                            }
                        }
                        WI[i][j][k].C_ptr = temp;
                        if (WI[i][j][k].ComponentSpace != 1 && WI[i][j][k].C_ptr.empty()) {
                            UsedSpace[i][j][k] = 0;
                        }
                    }
                }
            }
        }
        C.Result.clear();
        //  std::cout<<"\nSTARTFOUND wire status: "<<UsedSpace[12][1][1]<<"   "<<WI[12][1][1].ComponentSpace<<std::endl;
        //  if(!WI[12][1][1].C_ptr.empty()){
        //      for (auto it: WI[12][1][1].C_ptr)
        //     std::cout<<"\nSTARTFOUND wire name: "<<it->getName()<<std::endl;
        //  }else{
        //      std::cout<<"\nSTARTFOUND wire EMPTY"<<std::endl;
        //  }
    } else {
        // update usedspace and WI
        updateUsedSpace(C, Space);
    }
    return RetFlag;
}
