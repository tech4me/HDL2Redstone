#pragma once

#include <iostream>
#include <set>
#include <string>
#include <tuple>

#include <Component.hpp>

namespace HDL2Redstone {
class Connection {
  public:
    class Parameters {
      public:
        void SetParameters(std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points_);
        const std::vector<std::tuple<uint16_t, uint16_t, uint16_t>>& getParameters() const {
            return connection_points;
        };

      private:
        std::vector<std::tuple<uint16_t, uint16_t, uint16_t>> connection_points;
    };
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>> Result;
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_);
    bool getRouted() const { return Routed; }
    void setRouted(bool Routed_) { Routed = Routed_; }
    const std::string& getName() const;
    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>>& getPortConnection() const;
    // add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);

  private:
    bool Routed;
    const std::string Name;
    std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection;
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
