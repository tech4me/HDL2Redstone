#pragma once

#include <iostream>
#include <set>
#include <string>
#include <tuple>

#include <Cell.hpp>
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
    class ConnectionResult {
      public:
        ConnectionResult(std::tuple<uint16_t, uint16_t, uint16_t> coord_, const HDL2Redstone::Cell* CellPtr_,
                         HDL2Redstone::Orientation Ori_) {
            coord = coord_;
            CellPtr = CellPtr_;
            Ori = Ori_;
        };
        std::tuple<uint16_t, uint16_t, uint16_t> coord;
        const HDL2Redstone::Cell* CellPtr;
        HDL2Redstone::Orientation Ori;
    };
    struct resultcomp {
        bool operator()(const ConnectionResult& lhs, const ConnectionResult& rhs) const {
            return lhs.coord < rhs.coord;
        } // TODO: check repeater is higher level than wire, cant be replaced
    };
    std::set<ConnectionResult, resultcomp> Result;
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_, bool IsSource = true);
    bool getRouted() const { return Routed; }
    void setRouted(bool Routed_) { Routed = Routed_; }
    const std::string& getName() const { return Name; }
    const std::pair<Component*, std::string>& getSourcePortConnection() { return SourcePortConnection; }
    const std::vector<std::pair<Component*, std::string>>& getSinkPortConnections() { return SinkPortConnections; }
    // Add component port to source
    void addSource(Component* ComponentPtr_, const std::string& PortName_);
    // Add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> checkRouteResult();

  private:
    bool Routed;
    const std::string Name;
    std::pair<Component*, std::string> SourcePortConnection;
    std::vector<std::pair<Component*, std::string>> SinkPortConnections;
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
