#pragma once

#include <map>
#include <random>
#include <vector>

#include <Design.hpp>

namespace HDL2Redstone {
class Placer {
  public:
    Placer(Design& D_);
    bool place();

  private:
    struct Port {
        std::string Name;
        Direction Dir;
        Coordinate Coord;
        // Only set this up for input ports
        Port* ConnectedPort;
    };

    struct ComponentPlacementData {
        Component* ComponentPtr;
        Placement Place;
        bool Fixed;
        bool Empty;
        int GridX;
        int GridY;
        int GridZ;
        // All the ports belong in this component
        std::map<std::string, Port> Ports;
        void setPlacement(uint16_t X_, uint16_t Y_, uint16_t Z_, Orientation Orient_);
    };

    struct PlacementState {
        Design& D;
        std::vector<ComponentPlacementData> CPDs;
        // Placement grid: keep track of which cell is placed at which location
        std::vector<ComponentPlacementData*> PGrid;
        std::vector<ComponentPlacementData*> SwapCandidate1;
        std::vector<ComponentPlacementData*> SwapCandidate2;
        std::vector<std::unique_ptr<ComponentPlacementData>> EmptyCPDs;
        PlacementState(Design& D_);
        bool testAndSetPlacementGrid(ComponentPlacementData& CPD_);
        bool setPlacementGrid(ComponentPlacementData& CPD_);
    };

    void applyCurrentPlacement();
    bool initialPlace();
    bool annealPlace();
    bool checkLegality(bool SkipUnplaced_ = 0) const;
    double evalCost(const PlacementState& PS_) const;
    std::pair<int, int> annealGenerateSwapNeighbour();
    void annealDoSwap(int SwapFrom_, int SwapTo_);

    Design& D;
    PlacementState CurrentPlacement;
    double BestCost;
    int PGridW;
    int PGridH;
    int PGridL;
    std::random_device RD;
    std::mt19937_64 RGen;
    std::uniform_real_distribution<> AcceptGen;
};
} // namespace HDL2Redstone
