#pragma once

#include <map>
#include <unordered_map>
#include <vector>

#include <Design.hpp>

namespace HDL2Redstone {
class Placer {
  public:
    Placer(Design& D_);
    ~Placer();

    bool place();

  private:
    bool initialPlace();
    bool annealPlace();
    bool checkLegality(bool SkipUnplaced_ = 0) const;

    struct Coordination {
        uint16_t X;
        uint16_t Y;
        uint16_t Z;
    };

    struct Port {
        Coordination Coord;
        // List of connected ports
        std::vector<Port*> Ports;
    };

    struct ComponentPlacementData {
        Component* ComponentPtr;
        // All the ports belong in this component
        std::map<std::string, Port> Ports;
    };

    Design& D;
    std::unordered_map<Component*, ComponentPlacementData> ComponentMap;
    std::vector<std::vector<ComponentPlacementData*>> LayerCPD;
    // Legality matrix -- for keeping track of all the used space during placement
    int* UsedSpace;
    bool updateUsedSpace(const Component& C_, bool Status_);
    bool setUsedSpace(const Component& C_) { return updateUsedSpace(C_, true); }
    bool clearUsedSpace(const Component& C_) { return updateUsedSpace(C_, false); }
    bool testInitialPlacement(const Component& C_, uint16_t Clearance_) const;
};
} // namespace HDL2Redstone
