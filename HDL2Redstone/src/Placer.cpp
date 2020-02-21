#include <algorithm>
#include <sstream>

#include <Exception.hpp>
#include <Placer.hpp>

#define INITIAL_CLEARANCE 10

using namespace HDL2Redstone;

bool Placer::place() {
    if (checkLegality(true)) {
        std::cout << "Illegal user forced placement!" << std::endl;
        return true;
    }
    if (initialPlace()) {
        std::cout << "Initial placement failed, please consider allocate more space for design!" << std::endl;
        return true;
    }
    applyCurrentPlacement();
    if (checkLegality()) {
        throw Exception("Illegal initial placement!");
    }
    if (annealPlace()) {
        std::cout << "Simulated annealing placement failed!" << std::endl;
        return true;
    }
    applyCurrentPlacement();
    if (checkLegality()) {
        throw Exception("Illegal simulated annealing placement!");
    }
    return false;
}

void Placer::applyCurrentPlacement() {
    for (auto& CPD : CurrentPlacement.CPDs) {
        CPD.ComponentPtr->setPlacement(CPD.Place.X, CPD.Place.Y, CPD.Place.Z, CPD.Place.Orient);
    }
}

bool Placer::initialPlace() {
    std::cout << "Running initial placer..." << std::endl;
    for (auto& CPD : CurrentPlacement.CPDs) {
        const auto& Component = *(CPD.ComponentPtr);
        // Skip already placed component
        if (Component.getPlaced()) {
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
            CPD.setPlacement(X, Y, Z, Orientation::ZeroCW);
            if (!testInitialPlacement(CPD, INITIAL_CLEARANCE)) {
                CurrentPlacement.setUsedSpace(CPD);
                break;
            }
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
    return false;
}

bool Placer::annealPlace() {
    std::cout << "Running simulated annealing placer..." << std::endl;
    const int MaxIt = 1;
    const int MaxStep = 1;

    // Create initial state with initial placer result

    int i = 0;
    while (true) {
        if (i == MaxIt) {
            std::cout << "Annealer max number of iteration reached!" << std::endl;
            break;
        }
        /*
        double Tempreture = MaxIt / (i + 1);
        for (int j = 0; j < MaxStep; ++j) {
            // Generate neighbour solution
            NewState = annealGenerateNeighbour(State);
            // Accept probabilistic function
            if (annealAccept(State, NewState, Tempreture)) {
                State = NewState;
            }
        }
        */
        ++i;
    }

    // Save the final state
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
            return true;
        }
        for (int X = std::get<0>(P1); X != std::get<0>(P2); ++X) {
            for (int Y = std::get<1>(P1); Y != std::get<1>(P2); ++Y) {
                for (int Z = std::get<2>(P1); Z != std::get<2>(P2); ++Z) {
                    if (OccupiedSpace[X][Y][Z]) {
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

bool Placer::testInitialPlacement(const ComponentPlacementData& CPD_, uint16_t Clearance_) const {
    const auto& ComponentRange = CPD_.ComponentPtr->getRangeWithPlacement(CPD_.Place);
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    // Define search space
    uint16_t NewP1X = P1.X - Clearance_;
    uint16_t NewP1Y = P1.Y - Clearance_;
    uint16_t NewP1Z = P1.Z - Clearance_;
    uint16_t NewP2X = P2.X + Clearance_;
    uint16_t NewP2Y = P2.Y + Clearance_;
    uint16_t NewP2Z = P2.Z + Clearance_;
    if (NewP1X >= D.Width || NewP1Y >= D.Height || NewP1Z >= D.Length) {
        return true;
    }
    if (NewP2X >= D.Width || NewP2Y >= D.Height || NewP2Z >= D.Length) {
        return true;
    }
    for (int X = NewP1X; X != NewP2X; ++X) {
        for (int Y = NewP1Y; Y != NewP2Y; ++Y) {
            for (int Z = NewP1Z; Z != NewP2Z; ++Z) {
                if (CurrentPlacement.UsedSpace[X * D.Width * D.Height + Y * D.Width + Z]) {
                    return true;
                }
            }
        }
    }
    return false;
}

double Placer::evalCost() const { return 0; }

Placer::PlacementState::PlacementState(Design& D_) : D(D_) {
    UsedSpace = new int[D.Width * D.Height * D.Width];
    std::fill(UsedSpace, UsedSpace + D.Width * D.Height * D.Width, 0);
    // Create placement data structures
    const auto& Components = D.MN.getComponents();
    for (const auto& Component : Components) {
        ComponentPlacementData CPD;
        CPD.ComponentPtr = Component.get();
        if (Component->getPlaced()) {
            CPD.Place = Component->getPlacement();
        }
        for (const auto& PinName : Component->getPinNames()) {
            CPD.Ports.emplace(PinName, Port());
        }
        CPDs.push_back(CPD);
    }
    const auto& Connections = D.MN.getConnections();
    for (const auto& Connection : Connections) {
        const auto& SourcePort = Connection->getSourcePortConnection();
        const auto& SinkPorts = Connection->getSinkPortConnections();
        auto It1 = std::find_if(CPDs.begin(), CPDs.end(), [SourcePort](const ComponentPlacementData& CPD) {
            return CPD.ComponentPtr == SourcePort.first;
        });
        if (It1 != CPDs.end()) {
            for (const auto& SinkPort : SinkPorts) {
                auto It2 = std::find_if(CPDs.begin(), CPDs.end(), [SinkPort](const ComponentPlacementData& CPD) {
                    return CPD.ComponentPtr == SinkPort.first;
                });
                if (It2 != CPDs.end()) {
                    // CHECK: Only add info at the sink side
                    It2->Ports.at(SinkPort.second).ConnectedPort = &(It1->Ports.at(SourcePort.second));
                } else {
                    throw Exception("Components data and connections data doesn't match.");
                }
            }
        } else {
            throw Exception("Components data and connections data doesn't match.");
        }
    }

    // Read in user placed component and update used space
    for (const auto& CPD : CPDs) {
        if (CPD.ComponentPtr->getPlaced()) {
            setUsedSpace(CPD);
        }
    }
}

Placer::PlacementState::~PlacementState() { delete[] UsedSpace; }

bool Placer::PlacementState::updateUsedSpace(const ComponentPlacementData& CPD_, bool Status_) {
    const auto& ComponentRange = CPD_.ComponentPtr->getRangeWithPlacement(CPD_.Place);
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    if (P1.X >= D.Width || P1.Y >= D.Height || P1.Z >= D.Length) {
        return true;
    }
    if (P2.X > D.Width || P2.Y > D.Height || P2.Z > D.Length) {
        return true;
    }
    for (int X = P1.X; X != P2.X; ++X) {
        for (int Y = P1.Y; Y != P2.Y; ++Y) {
            for (int Z = P1.Z; Z != P2.Z; ++Z) {
                UsedSpace[X * D.Width * D.Height + Y * D.Width + Z] = Status_;
            }
        }
    }
    return false;
}
