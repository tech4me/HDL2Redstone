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

    struct WireInfo {
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
    bool regularRoute(Connection& C);
    bool reRouteStartRouting(coord congestionPoint);
    bool reRouteEndRouting(); // TODO
    bool reRouteIllegal(Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
                        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints_prev);
    bool reRouteIllegalHelper(
        Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& local_congestion_points,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& inserted_congestion_points);
    bool reRouteforIllegalRegularRouteHelper(
        Connection& C, std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& RetIllegalPoints,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t, uint16_t>>& congestionPoints,
        std::set<std::tuple<uint16_t, uint16_t, uint16_t>>& inserted_congestion_points);
    void checkUpdateGraph(uint16_t x, uint16_t y, uint16_t z,
                          std::priority_queue<Router::Point*, std::vector<Router::Point*>, Router::PointCompare>& Q,
                          Router::Point*& TempP);
    bool checkUpdateGraphHelper(Router::Point* Parent, Router::Point* Current);
    bool checkandKeepResult(Connection& C);
    bool checkKeepOrUpdate(Connection& C);
    void initPortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac);
    void initPortUsedSpaceHelper(const Coordinate& Coord);
    void updateUsedSpace(Connection& C);
    coord updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac, coord& congestion);
    void updateSinglePortUsedSpaceHelper(const Coordinate& Coord);
    bool routingLastRepeater(Router::Point* CongestionP);
    int routingLastRepeaterHelper(Router::Point* RecurP);

    inline int& getUsedSpace(uint16_t X_, uint16_t Y_, uint16_t Z_) {
        return UsedSpace[((X_ * D.Height) + Y_) * D.Width + Z_];
    }
    inline WireInfo& getWireInfo(uint16_t X_, uint16_t Y_, uint16_t Z_) {
        return WI[((X_ * D.Height) + Y_) * D.Width + Z_];
    }
    inline Point& getPoint(uint16_t X_, uint16_t Y_, uint16_t Z_) {
        return Points[((X_ * D.Height) + Y_) * D.Width + Z_];
    }

    Design& D;
    std::vector<int> UsedSpace;
    std::vector<WireInfo> WI;
    std::vector<Point> Points;
    Connection* FailedWireSingleRouting;
};
} // namespace HDL2Redstone