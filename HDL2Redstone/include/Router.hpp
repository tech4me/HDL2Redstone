#pragma once

#include <Design.hpp>

namespace HDL2Redstone {
class Router {
  public:
    bool flatRoute(Design& D, Connection& C);

  private:
    bool checkSingleRoute(const Design& D, const std::vector<std::tuple<int16_t, int16_t, int16_t>> connection_points);
    std::vector<std::tuple<int16_t, int16_t, int16_t>> flatRouteDirectLine(std::tuple<int16_t, int16_t, int16_t> start,
                                                                           std::tuple<int16_t, int16_t, int16_t> end);
};
} // namespace HDL2Redstone