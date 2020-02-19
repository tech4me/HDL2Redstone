#include <algorithm>
#include <sstream>

#include <Exception.hpp>
#include <Placer.hpp>

#define INITIAL_CLEARANCE 10

using namespace HDL2Redstone;

Placer::Placer(Design& D_) : D(D_) {
    UsedSpace = new int[D.Width * D.Height * D.Width];
    std::fill(UsedSpace, UsedSpace + D.Width * D.Height * D.Width, 0);
}

Placer::~Placer() { delete[] UsedSpace; }

bool Placer::place() {
    if (checkLegality(true)) {
        std::cout << "Illegal user forced placement!" << std::endl;
        return true;
    }
    if (initialPlace()) {
        std::cout << "Initial placement failed, please consider allocate more space for design!" << std::endl;
        return true;
    }
    if (checkLegality()) {
        throw Exception("Illegal initial placement!");
    }
    if (annealPlace()) {
        std::cout << "Simulated annealing placement failed!" << std::endl;
        return true;
    }
    if (checkLegality()) {
        throw Exception("Illegal simulated annealing placement!");
    }
    return false;
}

bool Placer::initialPlace() {
    std::cout << "Running initial placer..." << std::endl;
    // Create placement data structures
    const auto& Components = D.MN.getComponents();
    for (const auto& Component : Components) {
        ComponentPlacementData CPD;
        CPD.ComponentPtr = Component.get();
        for (const auto& PinName : Component->getPinNames()) {
            CPD.Ports.emplace(PinName, Port());
        }
        ComponentMap.emplace(Component.get(), CPD);
    }
    const auto& Connections = D.MN.getConnections();
    for (const auto& Connection : Connections) {
        const auto& SourcePort = Connection->getSourcePortConnection();
        const auto& SinkPorts = Connection->getSinkPortConnections();
        auto It1 = ComponentMap.find(SourcePort.first);
        if (It1 != ComponentMap.end()) {
            for (const auto& SinkPort : SinkPorts) {
                auto It2 = ComponentMap.find(SinkPort.first);
                if (It2 != ComponentMap.end()) {
                    // Add info at source side
                    It1->second.Ports.at(SourcePort.second).Ports.push_back(&(It2->second.Ports.at(SinkPort.second)));
                    // Add info at sink side
                    It2->second.Ports.at(SinkPort.second).Ports.push_back(&(It1->second.Ports.at(SourcePort.second)));
                } else {
                    throw Exception("Components data and connections data doesn't match.");
                }
            }
        } else {
            throw Exception("Components data and connections data doesn't match.");
        }
    }

    // Read in user placed component and update used space
    for (auto& Component : Components) {
        if (Component->getPlaced()) {
            setUsedSpace(*Component);
        }
    }

    for (auto& Component : Components) {
        // Skip already placed component
        if (Component->getPlaced()) {
            continue;
        }

        uint16_t X = 0;
        uint16_t Y = 0;
        uint16_t Z = 0;
        while (true) {
            if (Y >= D.Height) {
                std::cout << "DEBUG: Placement error: not enough space!" << std::endl;
                return true;
            }
            Component->setPlacement(X, Y, Z, Orientation::ZeroCW);
            if (!testInitialPlacement(*Component, INITIAL_CLEARANCE)) {
                setUsedSpace(*Component);
                break;
            }
            Component->clearPlacement();
            // Loop X and Z first
            ++X;
            if (X == D.Width) {
                X = 0;
                ++Z;
            }
            if (Z == D.Length) {
                X = 0;
                Z = 0;
                ++Y;
            }
        }
    }

    /*
    // Determine number of placement layers
    uint16_t NumLayer = std::get<1>(D.getSpace()) / LAYER_HEIGHT;

    // Assign layers
    // TODO: Add min k cut to seperate into layers
    uint16_t LayerSize = (ComponentMap.size() + NumLayer - 1) / NumLayer;
    uint16_t Counter = 0;
    uint16_t LayerNum = 0;
    LayerCPD.resize(NumLayer);
    for (auto& CPDPair : ComponentMap) {
        // Skip already placed component
        if (CPDPair.first->getPlaced()) {
            continue;
        }

        if (Counter < LayerSize) {
            LayerCPD.at(LayerNum).push_back(&(CPDPair.second));
            ++Counter;
        } else {
            Counter = 0;
            ++LayerNum;
        }
    }

    // Place each layer individually
    for (const auto& CPD : LayerCPD) {
        uint16_t Z_Layer = CPD.LayerNum * LAYER_HEIGHT;
        CPD;
    }
    */
    return false;
}

bool Placer::annealPlace() {
    std::cout << "Running simulated annealing placer..." << std::endl;
    return false;
}

bool Placer::updateUsedSpace(const Component& C_, bool Status_) {
    const auto& ComponentRange = C_.getRange();
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    if (std::get<0>(P1) >= D.Width || std::get<1>(P1) >= D.Height || std::get<2>(P1) >= D.Length) {
        return true;
    }
    if (std::get<0>(P2) > D.Width || std::get<1>(P2) > D.Height || std::get<2>(P2) > D.Length) {
        return true;
    }
    for (int X = std::get<0>(P1); X != std::get<0>(P2); ++X) {
        for (int Y = std::get<1>(P1); Y != std::get<1>(P2); ++Y) {
            for (int Z = std::get<2>(P1); Z != std::get<2>(P2); ++Z) {
                UsedSpace[X * D.Width * D.Height + Y * D.Width + Z] = Status_;
            }
        }
    }
    return false;
}

bool Placer::testInitialPlacement(const Component& C_, uint16_t Clearance_) const {
    const auto& ComponentRange = C_.getRange();
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    // Define search space
    uint16_t NewP1X = std::get<0>(P1) - Clearance_;
    uint16_t NewP1Y = std::get<1>(P1) - Clearance_;
    uint16_t NewP1Z = std::get<2>(P1) - Clearance_;
    uint16_t NewP2X = std::get<0>(P2) + Clearance_;
    uint16_t NewP2Y = std::get<1>(P2) + Clearance_;
    uint16_t NewP2Z = std::get<2>(P2) + Clearance_;
    if (NewP1X >= D.Width || NewP1Y >= D.Height || NewP1Z >= D.Length) {
        return true;
    }
    if (NewP2X >= D.Width || NewP2Y >= D.Height || NewP2Z >= D.Length) {
        return true;
    }
    for (int X = NewP1X; X != NewP2X; ++X) {
        for (int Y = NewP1Y; Y != NewP2Y; ++Y) {
            for (int Z = NewP1Z; Z != NewP2Z; ++Z) {
                if (UsedSpace[X * D.Width * D.Height + Y * D.Width + Z]) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool Placer::checkLegality(bool SkipUnplaced_) const {
    std::vector<std::vector<std::vector<bool>>> OccupiedSpace(
        D.Width, std::vector<std::vector<bool>>(D.Height, std::vector<bool>(D.Length)));
    const auto& Components = D.MN.getComponents();

    for (const auto& Component : Components) {
        // Skip the not yet placed or throw
        if (!Component->getPlaced()) {
            if (SkipUnplaced_) {
                continue;
            } else {
                std::stringstream S;
                S << "Unplaced component:" << std::endl;
                S << *Component;
                throw Exception(S.str());
            }
        }
        const auto& ComponentRange = Component->getRange();
        const auto& P1 = ComponentRange.first;
        const auto& P2 = ComponentRange.second;
        if (std::get<0>(P1) >= D.Width || std::get<1>(P1) >= D.Height || std::get<2>(P1) >= D.Length) {
            return true;
        }
        if (std::get<0>(P2) > D.Width || std::get<1>(P2) > D.Height || std::get<2>(P2) > D.Length) {
            std::cout << std::get<0>(P2) << " " << std::get<1>(P2) << " " << std::get<2>(P2) << std::endl;;
            return true;
        }
        for (int X = std::get<0>(P1); X != std::get<0>(P2); ++X) {
            for (int Y = std::get<1>(P1); Y != std::get<1>(P2); ++Y) {
                for (int Z = std::get<2>(P1); Z != std::get<2>(P2); ++Z) {
                    if (OccupiedSpace[X][Y][Z]) {
                        std::cout << X << " " << Y << " " << Z << std::endl;
                        return true;
                    } else {
                        OccupiedSpace[X][Y][Z] = true;
                    }
                }
            }
        }
    }
    return false;
}
