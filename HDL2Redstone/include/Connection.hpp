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
            if (std::get<0>(lhs.coord) == std::get<0>(rhs.coord) && std::get<1>(lhs.coord) == std::get<1>(rhs.coord) &&
                std::get<2>(lhs.coord) == std::get<2>(rhs.coord)) {
                return false;
            }
            return true;
        } // TODO: check repeater is higher level than wire, cant be replaced
    };
    std::set<ConnectionResult, resultcomp> Result;
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_);
    // TODO tempory inset to set, need to correct
    void setInsert(const ConnectionResult& CR_) {
        // if(Result.find(CR_)==Result.end()){
        // Result.insert(CR_);
        // }
        for (auto itt = Result.begin(); itt != Result.end(); ++itt) {
            if (CR_.coord == itt->coord) {
                return;
            }
        }
        Result.insert(CR_);
    };
    bool getRouted() const { return Routed; }
    void setRouted(bool Routed_) { Routed = Routed_; }
    const std::string& getName() const;
    const std::vector<std::tuple<Component*, std::string, Connection::Parameters>>& getPortConnection() const;
    // add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);
    std::set<std::tuple<uint16_t, uint16_t, uint16_t>> checkRouteResult();

  private:
    bool Routed;
    const std::string Name;
    std::vector<std::tuple<Component*, std::string, Connection::Parameters>> PortConnection;
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
