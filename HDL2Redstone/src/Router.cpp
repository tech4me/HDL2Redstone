#include <cmath>
#include <queue>
#include <set>

#include <Component.hpp>
#include <Connection.hpp>
#include <Router.hpp>

#define MAX_NUM_OF_WIRE 12

using namespace HDL2Redstone;
Router::coord Router::updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac, coord& congestion) {
    Router::coord ret;
    ret.x = std::get<0>(Loc);
    ret.y = std::get<1>(Loc);
    ret.z = std::get<2>(Loc);
    congestion.x = std::get<0>(Loc);
    congestion.y = std::get<1>(Loc);
    congestion.z = std::get<2>(Loc);
    if (Fac == Facing::North) {
        congestion.z--;
        if (UsedSpace[ret.x][ret.y][ret.z - 1] != 1) {
            UsedSpace[ret.x][ret.y][ret.z - 1] = 0;
            UsedSpace[ret.x][ret.y - 1][ret.z - 1] = 0;
            ret.z--;
            return ret;
        }
    }
    if (Fac == Facing::East) {
        congestion.x++;
        if (UsedSpace[ret.x + 1][ret.y][ret.z] != 1) {
            UsedSpace[ret.x + 1][ret.y][ret.z] = 0;
            UsedSpace[ret.x + 1][ret.y - 1][ret.z] = 0;
            ret.x++;
            return ret;
        }
    }
    // TODO: Add vertical pin
    if (Fac == Facing::South) {
        congestion.z++;
        if (UsedSpace[ret.x][ret.y][ret.z + 1] != 1) {
            UsedSpace[ret.x][ret.y][ret.z + 1] = 0;
            UsedSpace[ret.x][ret.y - 1][ret.z + 1] = 0;
            ret.z++;
            return ret;
        }
    }
    if (Fac == Facing::West) {
        congestion.x--;
        if (UsedSpace[ret.x - 1][ret.y][ret.z] != 1) {
            UsedSpace[ret.x - 1][ret.y][ret.z] = 0;
            UsedSpace[ret.x - 1][ret.y - 1][ret.z] = 0;
            ret.x--;
            return ret;
        }
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

    //add wire info for each block all over the space
    WI = new WireInfo**[std::get<0>(Space)];
    for (int i = 0; i < std::get<0>(Space); i++) {
        WI[i] = new WireInfo*[std::get<1>(Space)];
        for (int j = 0; j < std::get<1>(Space); j++) {
            WI[i][j] = new WireInfo[std::get<2>(Space)];
            for (int k = 0; k < std::get<2>(Space); k++) {
                WI[i][j][k].C_ptr = NULL;
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
    /*
        for (auto const& j : Connections_) {
            const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
                j->getPortConnection();
            for (auto const& k : PortConnection_) {
                if (!(std::get<2>(k).getParameters().empty())) {
                    UsedConnectionSpace.push_back(std::get<2>(k).getParameters());
                }
            }
        }
    */

    for (auto const& i : UsedComponentSpace) {
        for (uint16_t j = std::get<0>(i.first); j < std::get<0>(i.second); j++) {
            for (uint16_t k = std::get<1>(i.first); k < std::get<1>(i.second); k++) {
                for (uint16_t z = std::get<2>(i.first); z < std::get<2>(i.second); z++) {
                    UsedSpace[j][k][z] = 1;
                    if (j > 0)
                        UsedSpace[j - 1][k][z] = (UsedSpace[j - 1][k][z] == 1) ? 1 : 2;
                    if (j < std::get<0>(Space) - 1)
                        UsedSpace[j + 1][k][z] = (UsedSpace[j + 1][k][z] == 1) ? 1 : 2;

                    if (k > 0)
                        UsedSpace[j][k - 1][z] = (UsedSpace[j][k - 1][z] == 1) ? 1 : 2;
                    if (k < std::get<1>(Space) - 1)
                        UsedSpace[j][k + 1][z] =
                            (UsedSpace[j][k + 1][z] == 1) ? 1 : 2; // TODO veritcal pin maybe possible

                    if (z > 0)
                        UsedSpace[j][k][z - 1] = (UsedSpace[j][k][z - 1] == 1) ? 1 : 2;
                    if (z < std::get<2>(Space) - 1)
                        UsedSpace[j][k][z + 1] = (UsedSpace[j][k][z + 1] == 1) ? 1 : 2;
                }
            }
        }
    }
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
    for (auto& it : Connections_) {
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
        RegularRoute(D, *it, Space, P_);
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
void Router::checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z, Router::Point***& P_,
                              std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                              Router::Point*& TempP, std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    if ((!UsedSpace[x][y][z]) && (!UsedSpace[x][y - 1][z])) {
        // check the node is in for its potential path
        // check up to +- 2 unit are clean
        if ((P_[x][y][z].cost >= TempP->cost + 1)) {
            bool legal = 1;
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
                int set = 0;
                if (P_[x][y][z].cost == TempP->cost + 1) {
                    if (y == TempP->Loc.y) {
                        set = 1;
                        P_[x][y][z].P = TempP;
                    }
                } else {
                    set = 1;
                    P_[x][y][z].P = TempP;
                }
                if (set) {
                    if (TempP->length <= MAX_NUM_OF_WIRE) {
                        if (TempP->Loc.x > x) {
                            P_[x][y][z].ori = HDL2Redstone::Orientation::OneCW;
                        } else if (TempP->Loc.x < x) {
                            P_[x][y][z].ori = HDL2Redstone::Orientation::ThreeCW;
                        } else if (TempP->Loc.z > z) {
                            P_[x][y][z].ori = HDL2Redstone::Orientation::TwoCW;
                        } else if (TempP->Loc.z < z) {
                            P_[x][y][z].ori = HDL2Redstone::Orientation::ZeroCW;
                        }
                        P_[x][y][z].length = TempP->length + 1;
                    } else {
                        P_[x][y][z].length = 1;
                    }
                }
                P_[x][y][z].cost = TempP->cost + 1;
                if (!P_[x][y][z].visited && !P_[x][y][z].inserted) {
                    P_[x][y][z].inserted = 1;
                    Q.push(&P_[x][y][z]);
                }
            }
        }
    }
}
bool Router::RegularRoute(Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space,
                          Router::Point***& P_) {
    bool retFlag=true;
    // implement dijkstra
    Router::coord start;
    Router::coord congestionP;
    congestionP.x=-1;
    congestionP.y=-1;
    congestionP.z=-1;
    std::vector<Router::coord> end;
    std::tuple<uint16_t, uint16_t, uint16_t> startPin;
    // std::tuple<uint16_t, uint16_t, uint16_t> endPin;

    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
        C.getPortConnection();

    // source
    auto SrcFacing = (std::get<0>(PortConnection_[0]))->getPinFacing(std::get<1>(PortConnection_[0]));
    startPin = (std::get<0>(PortConnection_[0]))->getPinLocation(std::get<1>(PortConnection_[0]));
    start = Router::updateSinglePortUsedSpace(startPin, SrcFacing,congestionP);
    if (start.x == std::get<0>(startPin) && start.y == std::get<1>(startPin) && start.z == std::get<2>(startPin)) {
        std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z
                  << " because of other wires routing" << std::endl;
        std::cout << "congestion point is " << congestionP.x << ", " << congestionP.y << ", " << congestionP.z << std::endl;
        //if(!(congestionP.x==12 && congestionP.y==1 && congestionP.z==1)){
        if(!ReRouteStartRouting(congestionP, Space, P_, D)){
            return false;
        }
        //}
        //call update again TODO not good
        start = Router::updateSinglePortUsedSpace(startPin, SrcFacing,congestionP);
    }
    for (auto it = PortConnection_.begin() + 1; it != PortConnection_.end(); ++it) {
        auto temp = std::get<0>(*it)->getPinLocation(std::get<1>(*it));
        // endPin.push_back(temp);
        // temp = std::make_tuple(std::get<0>(temp),std::get<1>(temp),std::get<2>(temp) );
        auto endFace = (std::get<0>(*it))->getPinFacing(std::get<1>(*it));
        auto temp_result = Router::updateSinglePortUsedSpace(temp, endFace,congestionP);
        if (temp_result.x == std::get<0>(temp) && temp_result.y == std::get<1>(temp) &&
            temp_result.z == std::get<2>(temp)) {
            std::cout << "FAIL routing to " << temp_result.x << ", " << temp_result.y << ", " << temp_result.z
                      << " because of other wires routing" << std::endl;
            // if(!ReRouteStartRouting(congestionP, Space, P_, D)){
                 retFlag = false;
            //     temp_result = Router::updateSinglePortUsedSpace(temp, endFace,congestionP);
            // }else{
            //     temp_result = Router::updateSinglePortUsedSpace(temp, endFace,congestionP);
            //     end.push_back(temp_result); 
            // }
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

        if (TempP->Loc.x && (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW) &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x < std::get<0>(Space) - 1 &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW) &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW) &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z < std::get<2>(Space) - 1 &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW) &&
            (TempP->Loc.y) % 2) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }

        if (TempP->Loc.x && (TempP->Loc.y > 1) && (TempP->length <= MAX_NUM_OF_WIRE)) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.x && (TempP->Loc.y < std::get<1>(Space) - 1) &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::OneCW)) {
            Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y > 1) && (TempP->length <= MAX_NUM_OF_WIRE)) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y - 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.x < std::get<0>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1) &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ThreeCW)) {
            Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y + 1, TempP->Loc.z, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y > 1) && (TempP->length <= MAX_NUM_OF_WIRE)) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }
        if (TempP->Loc.z && (TempP->Loc.y < std::get<1>(Space) - 1) &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::TwoCW)) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z - 1, P_, Q, TempP, Space);
        }

        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y > 1) && (TempP->length <= MAX_NUM_OF_WIRE)) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y - 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
        if ((TempP->Loc.z < std::get<2>(Space) - 1) && (TempP->Loc.y < std::get<1>(Space) - 1) &&
            (TempP->length <= MAX_NUM_OF_WIRE || TempP->ori == HDL2Redstone::Orientation::ZeroCW)) {
            Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y + 1, TempP->Loc.z + 1, P_, Q, TempP, Space);
        }
    }
    // std::set<HDL2Redstone::Connection::ConnectionResult,HDL2Redstone::Connection::resultcomp> Result;
    C.setRouted(0);
    // if (endTemp.empty()) {
    Point* ptr = NULL;
    for (auto it : end) {
        if (P_[it.x][it.y][it.z].P) {
            C.setRouted(1);
            uint16_t TempX, TempY, TempZ;
            TempX = it.x;
            TempY = it.y;
            TempZ = it.z;
if (((TempX) == 13 && (TempY == 4) && TempZ == 5)/* || ((TempX) == 11 && (TempY == 1) && TempZ == 14)||((TempX) == 9 && (TempY == 4) && TempZ == 1)
|| ((TempX) == 12 && (TempY == 4) && TempZ == 1)*/) {
    if(WI[13][4][5].C_ptr){
std::cout << "\nXXXX4XX routing to " <<WI[13][4][5].C_ptr->getName() << std::endl;
    }
    std::cout << "\nXX4XXXDETECT routing to " << TempX << ", " << TempY << ", " << TempZ << std::endl;
}
if (((TempX) == 13 && (TempY == 5) && TempZ == 5)/* || ((TempX) == 11 && (TempY == 1) && TempZ == 14)||((TempX) == 9 && (TempY == 4) && TempZ == 1)
|| ((TempX) == 12 && (TempY == 4) && TempZ == 1)*/) {
    if(WI[13][5][5].C_ptr){
std::cout << "\nXXX5XXX routing to " <<WI[13][5][5].C_ptr->getName() << std::endl;
    }
    std::cout << "\nXXX5XXDETECT routing to " << TempX << ", " << TempY << ", " << TempZ << std::endl;
    //goto next;
}
            if (P_[it.x][it.y][it.z].length >= MAX_NUM_OF_WIRE + 1) {
                C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                         D.CellLib.getCellPtr("BUF"), P_[it.x][it.y][it.z].ori));
            } else {
                C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                         D.CellLib.getCellPtr("WIRE"), P_[it.x][it.y][it.z].ori));
            }
            //WI[TempX][TempY][TempZ].C_ptr = &C;
            ptr = P_[it.x][it.y][it.z].P;
            int i = 0;
            while (ptr != NULL) {
                uint16_t TempX, TempY, TempZ;
                TempX = ptr->Loc.x;
                TempY = ptr->Loc.y;
                TempZ = ptr->Loc.z;
std::cout << "i: "<<i<<" "<<TempX << ", " << TempY << ", " << TempZ <<"cost: "<<ptr->cost<< std::endl;
i++;
if (((TempX) == 13 && (TempY == 4) && TempZ == 5)/* || ((TempX) == 11 && (TempY == 1) && TempZ == 14)||((TempX) == 9 && (TempY == 4) && TempZ == 1)
|| ((TempX) == 12 && (TempY == 4) && TempZ == 1)*/) {
if(WI[13][4][5].C_ptr){
std::cout << "\nin 4 while routing to " <<WI[13][4][5].C_ptr->getName() << std::endl;
    }
    std::cout << "\nin 4 while routing to " << TempX << ", " << TempY << ", " << TempZ << std::endl;
    //goto next;
}
if (((TempX) == 13 && (TempY == 5) && TempZ == 5)/* || ((TempX) == 11 && (TempY == 1) && TempZ == 14)||((TempX) == 9 && (TempY == 4) && TempZ == 1)
|| ((TempX) == 12 && (TempY == 4) && TempZ == 1)*/) {
    if(WI[13][5][5].C_ptr){
std::cout << "\nin 5 while routing to " <<WI[13][5][5].C_ptr->getName() << std::endl;
    }
    std::cout << "\nin 5 while routing to " << TempX << ", " << TempY << ", " << TempZ << std::endl;
    //goto next;
}
                if (ptr->length >= MAX_NUM_OF_WIRE + 1) {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("BUF"), ptr->ori));
                } else {
                    C.setInsert(Connection::ConnectionResult(std::make_tuple(TempX, TempY - 1, TempZ),
                                                             D.CellLib.getCellPtr("WIRE"), ptr->ori));
                }
                next:;
                //WI[TempX][TempY][TempZ].C_ptr = &C;
                ptr = ptr->P;
            }
        } else {
            std::cout << "FAIL routing from " << start.x << ", " << start.y << ", " << start.z << " to " << it.x << ", "
                      << it.y << ", " << it.z << std::endl;
            retFlag = false;
        }
    }

    //     for(auto itt = C.Result.begin(); itt!=C.Result.end(); ++itt)
    //     {
    // if(std::get<0>(itt->coord) == 20 && (std::get<1>(itt->coord) == 2) &&
    // std::get<2>(itt->coord) == 1)
    //         std::cout << "in while SUCCESS routing to "
    //                      <<std::get<0>(itt->coord)<<", "<<std::get<1>(itt->coord)<<", "<<std::get<2>(itt->coord)<<std::endl;
    //     }
    
    updateUsedSpace(C, Space);

    // debugging print
    /*
        for (int i = 0; i < std::get<0>(Space); i++) {
            for (int j = 0; j < std::get<1>(Space); j++) {
                for (int k = 0; k < std::get<2>(Space); k++) {
                    if (j == 1)
                        std::cout << UsedSpace[i][j][k] << " ";
                }
            }
            std::cout << std::endl;
        }
    */
    // C.Result = Result;
    return retFlag;
}
// bool Router::flatRoute(Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space,
//                        Router::Point***& P_) {
//     // implement dijkstra
//     Router::coord start;
//     std::vector<Router::coord> end;
//     std::tuple<uint16_t, uint16_t, uint16_t> startPin;
//     // std::tuple<uint16_t, uint16_t, uint16_t> endPin;

//     const std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection_ =
//         C.getPortConnection();

//     // source
//     auto SrcFacing = (std::get<0>(PortConnection_[0]))->getPinFacing(std::get<1>(PortConnection_[0]));
//     startPin = (std::get<0>(PortConnection_[0]))->getPinLocation(std::get<1>(PortConnection_[0]));
//     start = Router::updateSinglePortUsedSpace(startPin, SrcFacing);
//     for (auto it = PortConnection_.begin() + 1; it != PortConnection_.end(); ++it) {
//         auto temp = std::get<0>(*it)->getPinLocation(std::get<1>(*it));
//         // endPin.push_back(temp);
//         // temp = std::make_tuple(std::get<0>(temp),std::get<1>(temp),std::get<2>(temp) );
//         auto endFace = (std::get<0>(*it))->getPinFacing(std::get<1>(*it));
//         end.push_back(Router::updateSinglePortUsedSpace(temp, endFace));
//     }
//     std::vector<Router::coord> endTemp = end;
//     std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare> Q;
//     P_[start.x][start.y][start.z].cost = 0;
//     P_[start.x][start.y][start.z].inserted = 1;
//     Q.push(&P_[start.x][start.y][start.z]);
//     bool done = 0;

//     while (!Q.empty()) {
//         Router::Point* TempP = Q.top();
//         Q.pop();
//         TempP->visited = 1;
//         for (auto it = endTemp.begin(); it != endTemp.end(); ++it) {
//             if ((TempP->Loc.x == it->x) && (TempP->Loc.y == it->y) && (TempP->Loc.z == it->z)) {
//                 // std::cout <<"Location: "<<std::get<0>(TempP->Loc)<<" "<<std::get<1>(TempP->Loc)<<"
//                 // "<<std::get<2>(TempP->Loc)<<std::endl;
//                 endTemp.erase(it);
//                 break;
//             }
//         }
//         if (endTemp.empty())
//             break;
//         if (TempP->Loc.x) {
//             Router::checkUpdateGraph(TempP->Loc.x - 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
//         }
//         if (TempP->Loc.x < std::get<0>(Space) - 1) {
//             Router::checkUpdateGraph(TempP->Loc.x + 1, TempP->Loc.y, TempP->Loc.z, P_, Q, TempP, Space);
//         }
//         if (TempP->Loc.z) {
//             Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z - 1, P_, Q, TempP, Space);
//         }
//         if (TempP->Loc.z < std::get<2>(Space) - 1) {
//             Router::checkUpdateGraph(TempP->Loc.x, TempP->Loc.y, TempP->Loc.z + 1, P_, Q, TempP, Space);
//         }
//     }
//     std::set<std::tuple<uint16_t, uint16_t, uint16_t>> Result;
//     if (endTemp.empty()) {
//         Point* ptr = NULL;
//         for (auto it : end) {
//             Result.insert(std::make_tuple(it.x, it.y - 1, it.z));
//             ptr = P_[it.x][it.y][it.z].P;
//             while (ptr != NULL) {
//                 Result.insert(std::make_tuple(ptr->Loc.x, ptr->Loc.y - 1, ptr->Loc.z));
//                 ptr = ptr->P;
//             }
//         }
//     }
//     updateUsedSpace(Result, Space);

//     // debugging print
//     /*
//         for (int i = 0; i < std::get<0>(Space); i++) {
//             for (int j = 0; j < std::get<1>(Space); j++) {
//                 for (int k = 0; k < std::get<2>(Space); k++) {
//                     if (j == 1)
//                         std::cout << UsedSpace[i][j][k] << " ";
//                 }
//             }
//             std::cout << std::endl;
//         }
//     */
//     C.Result = Result;
//     return true;
// }
void Router::updateUsedSpace(Connection& C,
                             std::tuple<uint16_t, uint16_t, uint16_t>& Space) {
    std::set<Connection::ConnectionResult, Connection::resultcomp>& Result = C.Result;
    for (const auto& entry : Result) {
        Router::coord entry_temp;
        entry_temp.x = std::get<0>(entry.coord);
        entry_temp.y = std::get<1>(entry.coord);
        entry_temp.z = std::get<2>(entry.coord);
        UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z] = 1;
        WI[entry_temp.x][entry_temp.y + 1][entry_temp.z].C_ptr = &C;
        UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z] = 1;
        WI[entry_temp.x][entry_temp.y][entry_temp.z].C_ptr = &C;
        if (entry_temp.y > 0) {
            UsedSpace[entry_temp.x][entry_temp.y - 1][entry_temp.z] = 1;
            WI[entry_temp.x][entry_temp.y - 1][entry_temp.z].C_ptr = &C;
        }
        if (entry_temp.y < std::get<1>(Space) - 2) {
            UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z] = 1;
            WI[entry_temp.x][entry_temp.y + 2][entry_temp.z].C_ptr = &C;
        }
        if (entry_temp.x > 0) {
            UsedSpace[entry_temp.x - 1][entry_temp.y + 1][entry_temp.z] = 1;
            WI[entry_temp.x - 1][entry_temp.y + 1][entry_temp.z].C_ptr = &C;
            UsedSpace[entry_temp.x - 1][entry_temp.y][entry_temp.z] = 1;
            WI[entry_temp.x - 1][entry_temp.y][entry_temp.z].C_ptr = &C;
            if (entry_temp.y < std::get<1>(Space) - 2) {
                UsedSpace[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z] = 1;
                WI[entry_temp.x - 1][entry_temp.y + 2][entry_temp.z].C_ptr = &C;
            }
        }
        if (entry_temp.x < std::get<0>(Space) - 1) {
            UsedSpace[entry_temp.x + 1][entry_temp.y + 1][entry_temp.z] = 1;
            WI[entry_temp.x + 1][entry_temp.y + 1][entry_temp.z].C_ptr = &C;
            UsedSpace[entry_temp.x + 1][entry_temp.y][entry_temp.z] = 1;
            WI[entry_temp.x + 1][entry_temp.y][entry_temp.z].C_ptr = &C;
            if (entry_temp.y < std::get<1>(Space) - 2) {
                UsedSpace[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z] = 1;
                WI[entry_temp.x + 1][entry_temp.y + 2][entry_temp.z].C_ptr = &C;
            }
        }
        if (entry_temp.z > 0) {
            UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z - 1] = 1;
            WI[entry_temp.x][entry_temp.y + 1][entry_temp.z - 1].C_ptr = &C;
            UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z - 1] = 1;
            WI[entry_temp.x][entry_temp.y][entry_temp.z - 1].C_ptr = &C;
            if (entry_temp.y < std::get<1>(Space) - 2) {
                UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1] = 1;
                WI[entry_temp.x][entry_temp.y + 2][entry_temp.z - 1].C_ptr = &C;
            }
        }
        if (entry_temp.z < std::get<2>(Space) - 1) {
            UsedSpace[entry_temp.x][entry_temp.y + 1][entry_temp.z + 1] = 1;
            WI[entry_temp.x][entry_temp.y + 1][entry_temp.z + 1].C_ptr = &C;
            UsedSpace[entry_temp.x][entry_temp.y][entry_temp.z + 1] = 1;
            WI[entry_temp.x][entry_temp.y][entry_temp.z + 1].C_ptr = &C;
            if (entry_temp.y < std::get<1>(Space) - 2) {
                UsedSpace[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1] = 1;
                WI[entry_temp.x][entry_temp.y + 2][entry_temp.z + 1].C_ptr = &C;
            }
        }
// if (((entry_temp.x) == 13 && (entry_temp.y == 4) && entry_temp.z == 5)/* || ((TempX) == 11 && (TempY == 1) && TempZ == 14)||((TempX) == 9 && (TempY == 4) && TempZ == 1)
// || ((TempX) == 12 && (TempY == 4) && TempZ == 1)*/) {
//     std::cout << "\n\n12 2 5::: " << UsedSpace[13][4][5] << std::endl;
//     }
    }
}
bool Router::ReRouteStartRouting(coord congestionPoint, std::tuple<uint16_t, uint16_t, uint16_t>& Space,Router::Point***& P_, Design& D){
    auto C_ptr = WI[congestionPoint.x][congestionPoint.y][congestionPoint.z].C_ptr;
    if(!C_ptr){
        std::cout<<"ReRoute Failed: congest each other"<<std::endl;
        return false;
    }
    std::string rollback_wire_name = WI[congestionPoint.x][congestionPoint.y][congestionPoint.z].C_ptr->getName();
    //roll back congestion wires
    for (int i = 0; i < std::get<0>(Space); i++) {
        for (int j = 0; j < std::get<1>(Space); j++) {
            for (int k = 0; k < std::get<2>(Space); k++) {
                if(WI[i][j][k].C_ptr && WI[i][j][k].C_ptr->getName() == rollback_wire_name){
                    UsedSpace[i][j][k] = 0;
                    WI[i][j][k].C_ptr->Result.clear();
                    WI[i][j][k].C_ptr = NULL;
                }
            }
        }
    }
    //force congestion point is occupied
    UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z] = 3;
    if (congestionPoint.y < std::get<1>(Space) - 1){
        if(UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z]==0){
            UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] = 3;
        }
    } 
    if (congestionPoint.y > 0) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z]==0){
            UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.x > 0) {
        if(UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z]==0){
            UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.x < std::get<0>(Space) - 1) { 
        if(UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z]==0){
            UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] = 3;
        }
    }
    if (congestionPoint.z > 0) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1]==0){
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] = 3;
        }
    }
    if (congestionPoint.z < std::get<2>(Space) - 1) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1]==0){
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] = 3;
        }
    }
    //route congested wire again
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
    std::cout << "ReRouting: " << rollback_wire_name << std::endl;
    bool ReRouteFlag = RegularRoute(D, *C_ptr, Space, P_);
    if(UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z]==3){
        UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z] = 0;
    }
    if (congestionPoint.y < std::get<1>(Space) - 1){
        if(UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z]==3){
            UsedSpace[congestionPoint.x][congestionPoint.y + 1][congestionPoint.z] = 0;
        }
    } 
    if (congestionPoint.y > 0) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z]==3){
            UsedSpace[congestionPoint.x][congestionPoint.y - 1][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.x > 0) {
        if(UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z]==3){
            UsedSpace[congestionPoint.x - 1][congestionPoint.y][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.x < std::get<0>(Space) - 1) { 
        if(UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z]==3){
            UsedSpace[congestionPoint.x + 1][congestionPoint.y][congestionPoint.z] = 0;
        }
    }
    if (congestionPoint.z > 0) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1]==3){
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z - 1] = 0;
        }
    }
    if (congestionPoint.z < std::get<2>(Space) - 1) {
        if(UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1]==3){
            UsedSpace[congestionPoint.x][congestionPoint.y][congestionPoint.z + 1] = 0;
        }
    }
    //roll back P_
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
    if(ReRouteFlag){
        std::cout << "ReRouting Success: " << rollback_wire_name<< std::endl;
        return true;
    }
    std::cout << "ReRouting Failed: " << rollback_wire_name<< std::endl;
    return false;
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
