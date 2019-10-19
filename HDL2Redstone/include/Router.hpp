#pragma once

#include <Design.hpp>

namespace HDL2Redstone {
class Router {
  public:
    Router(const Design& D);
    bool flatRoute(Design& D, Connection& C);

  private:
    bool checkSingleRoute(const Design& D,
                          const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points);
    bool checkPointAvaliable(const Design& D, const std::tuple<int16_t, int16_t, int16_t> location);
    std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>
    flatRouteDirectLine(std::tuple<uint16_t, uint16_t, uint16_t> start, std::tuple<uint16_t, uint16_t, uint16_t> end);
    bool updateUsedSpace(Connection& C, std::tuple<uint16_t, uint16_t, uint16_t> bound);
    void updateSinglePortUsedSpace(std::tuple<uint16_t, uint16_t, uint16_t> Loc, Facing Fac);
    bool*** UsedSpace;
};
} // namespace HDL2Redstone