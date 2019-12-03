#pragma once
#include <queue>

#include <Design.hpp>
#include <Placement.hpp>

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
    class Point {
      public:
        coord Loc;
        int cost;
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
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>
    flatRouteDirectLine(std::tuple<uint16_t, uint16_t, uint16_t> start, std::tuple<uint16_t, uint16_t, uint16_t> end);
    void updateUsedSpace(std::set<Connection::ConnectionResult,Connection::resultcomp>& Result,
                             std::tuple<uint16_t, uint16_t, uint16_t>& Space);
    coord updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac);
    bool*** UsedSpace;
};
} // namespace HDL2Redstone