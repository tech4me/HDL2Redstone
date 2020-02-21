#pragma once
#include <queue>

#include <Design.hpp>
#include <Placement.hpp>
#define MAX_NUM_OF_WIRE 12
#define MUL_COST_INC 1
#define BASE_COST_INC 1
namespace HDL2Redstone {
class Router {
  public:
    Router(const Design& D);
    void route(Design& D);

  private:
    typedef struct {
        uint16_t x;
        uint16_t y;
        uint16_t z;
    } coord;
    typedef struct {
        int ComponentSpace;
        std::set<Connection*> C_ptr;
    } WireInfo;
    class Point {
      public:
        coord Loc;
        int cost;
        int base_cost;
        int mul_cost;
        int length;
        Point* P;
        bool visited;
        bool inserted;
        HDL2Redstone::Orientation ori;
    };
    class PointCompare {
      public:
        bool operator()(const Point* lhs, const Point* rhs) const { return lhs->cost > rhs->cost; }
    };
    void checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z, Router::Point***& P_,
                          std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                          Router::Point*& TempP, std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    bool RegularRoute(Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_);
    bool flatRoute(Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_);
    bool checkSingleRoute(const Design& D,
                          const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points);
    bool checkPointAvaliable(const Design& D, const std::tuple<int16_t, int16_t, int16_t> location);
    void updateUsedSpace(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    coord updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac, coord& congestion,
                                    std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    bool ReRouteStartRouting(coord congestionPoint, std::tuple<uint16_t, uint16_t, uint16_t>& Space,
                             Router::Point***& P_, Design& D);
    bool ReRouteEndRouting(); // TODO
    bool ReRouteIllegal(Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
                        std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_, Design& D,
                        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints_prev);
    bool HelperReRouteIllegal(
        Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& local_congestion_points,
        std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_, Design& D);
    bool HelperReRouteforIllegalRegularRoute(
        Design& D, Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& RetIllegalPoints,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints);
    void Deconstructor(std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    void Reconstructor(const Design& D);
    bool CheckandKeepResult(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    bool CheckKeepOrUpdate(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t>& Space, Router::Point***& P_);
    bool HelperCheckUpdateGraph(Router::Point* Parent, Router::Point* Current);
    int HelperRoutingLastRepeater(Router::Point* RecurP);
    bool RoutingLastRepeater(Router::Point* CongestionP);
    void InitPortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac,
                           std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    void InitPortUsedSpaceHelper(Router::coord& Loc, std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    void updateSinglePortUsedSpaceHelper(Router::coord& Loc, std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    int*** UsedSpace;
    WireInfo*** WI;
    Connection* FailedWire_SingleRouting;
};
} // namespace HDL2Redstone
