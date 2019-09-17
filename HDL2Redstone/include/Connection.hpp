#pragma once

#include <iostream>
#include <string>
#include <tuple>

#include <Component.hpp>

namespace HDL2Redstone {
class Connection {
  public:
    class Parameters {
      public:
        void SetParameters(std::vector<std::tuple<int16_t, int16_t, int16_t>> connection_points_);

      private:
        std::vector<std::tuple<int16_t, int16_t, int16_t>> connection_points;
    };

    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_);
    const std::string& getName() const;
    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>>& getPortConnection() const;
    // add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);

  private:
    const std::string Name;
    std::vector<std::tuple<Component*, std::string, Parameters>> PortConnection;
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
