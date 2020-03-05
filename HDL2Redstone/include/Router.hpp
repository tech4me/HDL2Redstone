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
    Router(Design& D_);
    bool route();

  private:
    typedef struct {
        uint16_t x;
        uint16_t y;
        uint16_t z;
    } coord;

    struct WireInfo{
        int ComponentSpace = 0;
        std::set<Connection*> CPtrs;
    };

    struct Point {
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

    void initStructs();
    void checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z, Router::Point***& P_,
                          std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                          Router::Point*& TempP);
    bool RegularRoute(Design& D, Connection& C, Router::Point***& P_);
    bool flatRoute(Design& D, Connection& C, Router::Point***& P_);
    bool checkSingleRoute(const Design& D,
                          const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points);
    bool checkPointAvaliable(const Design& D, const std::tuple<int16_t, int16_t, int16_t> location);
    void updateUsedSpace(Connection& C);
    bool reRouteStartRouting(coord congestionPoint,
                             Router::Point***& P_, Design& D);
    bool ReRouteEndRouting(); // TODO
    bool reRouteIllegal(Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
                        Router::Point***& P_, Design& D,
                        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints_prev);
    bool reRouteIllegalHelper(
        Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& local_congestion_points,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>&inserted_congestion_points, Router::Point***& P_, Design& D);
    bool reRouteforIllegalRegularRouteHelper(
        Design& D, Connection& C, Router::Point***& P_,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& RetIllegalPoints,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>&inserted_congestion_points);
    bool CheckandKeepResult(Connection& C);
    bool CheckKeepOrUpdate(Connection& C, Router::Point***& P_);
    bool HelperCheckUpdateGraph(Router::Point* Parent, Router::Point* Current);
    int HelperRoutingLastRepeater(Router::Point* RecurP);
    bool RoutingLastRepeater(Router::Point* CongestionP);
    void initPortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac);
    void initPortUsedSpaceHelper(const Coordinate& Coord);
    coord updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac, coord& congestion);
    void updateSinglePortUsedSpaceHelper(const Coordinate& Coord);

    inline int& getUsedSpace(uint16_t X_, uint16_t Y_, uint16_t Z_) {
        return UsedSpace[((X_ * D.Height) + Y_) * D.Width + Z_];
    }
    inline WireInfo& getWireInfo(uint16_t X_, uint16_t Y_, uint16_t Z_) {
        return WI[((X_ * D.Height) + Y_) * D.Width + Z_];
    }

    Design& D;
    std::vector<int> UsedSpace;
    std::vector<WireInfo> WI;
    Connection* FailedWireSingleRouting;
};
} // namespace HDL2Redstone