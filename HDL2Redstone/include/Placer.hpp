#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include <Design.hpp>

namespace HDL2Redstone {
class Placer {
  public:
    Placer(Design& D_) : D(D_), CurrentPlacement(D_){};
    bool place();

  private:
    struct Port {
        Direction Dir;
        Coordinate Coord;
        // Only set this up for input ports
        Port* ConnectedPort;
    };

    struct ComponentPlacementData {
        Component* ComponentPtr;
        Placement Place;
        // All the ports belong in this component
        std::map<std::string, Port> Ports;
        void setPlacement(uint16_t X_, uint16_t Y_, uint16_t Z_, Orientation Orient_) {
            Place.X = X_;
            Place.Y = Y_;
            Place.Z = Z_;
            Place.Orient = Orient_;
        };
    };

    struct PlacementState {
        Design& D;
        std::vector<ComponentPlacementData> CPDs;
        // Legality matrix -- for keeping track of all the used space during placement
        int* UsedSpace;
        PlacementState(Design& D_);
        ~PlacementState();
        bool updateUsedSpace(const ComponentPlacementData& CPD_, bool Status_);
        bool setUsedSpace(const ComponentPlacementData& CPD_) { return updateUsedSpace(CPD_, true); }
        bool clearUsedSpace(const ComponentPlacementData& CPD_) { return updateUsedSpace(CPD_, false); }
    };

    void applyCurrentPlacement();
    bool initialPlace();
    bool annealPlace();
    bool checkLegality(bool SkipUnplaced_ = 0) const;
    bool testInitialPlacement(const ComponentPlacementData& CPD_, uint16_t Clearance_) const;
    double evalCost() const;

    Design& D;
    PlacementState CurrentPlacement;
};
} // namespace HDL2Redstone
