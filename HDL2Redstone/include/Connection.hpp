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
                         HDL2Redstone::Orientation Ori_, int SinkId_ = -1) {
            coord = coord_;
            CellPtr = CellPtr_;
            Ori = Ori_;
            SinkId = SinkId_;
        };
        ConnectionResult(const ConnectionResult& CRx) {
            coord = CRx.coord;
            CellPtr = CRx.CellPtr;
            Ori = CRx.Ori;
            SinkId = CRx.SinkId;
        }
        std::tuple<uint16_t, uint16_t, uint16_t> coord;
        const HDL2Redstone::Cell* CellPtr;
        HDL2Redstone::Orientation Ori;
        int SinkId; // for timing
    };
    struct resultcomp {
        bool operator()(const ConnectionResult& lhs, const ConnectionResult& rhs) const {
            // if (std::get<0>(lhs.coord) == std::get<0>(rhs.coord) && std::get<1>(lhs.coord) == std::get<1>(rhs.coord)
            // &&
            //     std::get<2>(lhs.coord) == std::get<2>(rhs.coord)) {
            //     return false;
            // }
            if (std::get<0>(lhs.coord) < std::get<0>(rhs.coord)) {
                return true;
            } else if (std::get<0>(lhs.coord) > std::get<0>(rhs.coord)) {
                return false;
            } else {
                if (std::get<1>(lhs.coord) < std::get<1>(rhs.coord)) {
                    return true;
                } else if (std::get<1>(lhs.coord) > std::get<1>(rhs.coord)) {
                    return false;
                } else {
                    if (std::get<2>(lhs.coord) < std::get<2>(rhs.coord)) {
                        return true;
                    } else if (std::get<2>(lhs.coord) > std::get<2>(rhs.coord)) {
                        return false;
                    } else {
                        if (lhs.Ori < rhs.Ori) {
                            return true;
                        } else if (lhs.Ori > rhs.Ori) {
                            return false;
                        } else {
                            return false;
                        }
                    }
                }
            }
        } // TODO: check repeater is higher level than wire, cant be replaced
    };
    std::set<ConnectionResult, resultcomp> Result;
    std::set<ConnectionResult, resultcomp> RouteResult;
    void calculateDelay();
    std::vector<int> getSinkDelays() { return SinkDelays; }
    Connection(const std::string& Name_, Component* ComponentPtr_, const std::string& PortName_, bool IsSource = true);
    void wireAlign(uint16_t& x, uint16_t& z, Orientation ori);
    bool getRouted() const { return Routed; }
    void setRouted(bool Routed_) { Routed = Routed_; }
    const std::string& getName() const { return Name; }
    const std::pair<Component*, std::string>& getSourcePortConnection() { return SourcePortConnection; }
    const std::vector<std::pair<Component*, std::string>>& getSinkPortConnections() { return SinkPortConnections; }
    // Add component port to source
    void addSource(Component* ComponentPtr_, const std::string& PortName_);
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
        ConnectionResult CR_temp(CR_);
        wireAlign(std::get<0>(CR_temp.coord), std::get<2>(CR_temp.coord), CR_.Ori);
        RouteResult.insert(CR_temp);
    };

    int getUnableRouting() const { return Unable_Routing; }
    void setUnableRouting(int Unable_Routing_) { Unable_Routing = Unable_Routing_; }
    // add component port to sink
    void addSink(Component* ComponentPtr_, const std::string& PortName_);
    std::set<std::tuple<uint16_t, uint16_t, uint16_t, uint16_t>> checkRouteResult();

  private:
    int Unable_Routing;
    bool Routed;
    const std::string Name;
    std::pair<Component*, std::string> SourcePortConnection;
    std::vector<std::pair<Component*, std::string>> SinkPortConnections;
    std::vector<int> SinkDelays; // for timing purpose
    friend std::ostream& operator<<(std::ostream& out, const Connection& Connection_);
};

} // namespace HDL2Redstone
