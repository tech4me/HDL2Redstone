#include <algorithm>
#include <cmath>
#include <sstream>

#include <Exception.hpp>
#include <Placer.hpp>

#define PLACEMENT_GRID_SIZE 10

using namespace HDL2Redstone;

Placer::Placer(Design& D_) : D(D_), CurrentPlacement(D_), RGen(RD()), AcceptGen(0.0, 1.0) {
    BestCost = std::numeric_limits<double>::max();
    PGridW = D.Width / PLACEMENT_GRID_SIZE;
    PGridH = D.Height / PLACEMENT_GRID_SIZE;
    PGridL = D.Length / PLACEMENT_GRID_SIZE;
}

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
        if (Component.getPlaced()) {
            auto Place = Component.getPlacement();
            CPD.setPlacement(Place.X, Place.Y, Place.Z, Place.Orient);
            CPD.Fixed = true;
            if (CurrentPlacement.setPlacementGrid(CPD)) {
                throw Exception("Illegal forced placement!");
            }
        }
    }
    for (auto& CPD : CurrentPlacement.CPDs) {
        const auto& Component = *(CPD.ComponentPtr);
        // Skip placing already placed component
        if (Component.getPlaced()) {
            continue;
        }

        uint16_t GridX = 0;
        uint16_t GridY = 0;
        uint16_t GridZ = 0;
        while (true) {
            if (GridY >= D.Height / PLACEMENT_GRID_SIZE) {
                std::cout << "DEBUG: Placement error: not enough space!" << std::endl;
                return true;
            }
            // TODO: Handle cells that requires multiple grid blocks
            uint16_t CW = CPD.ComponentPtr->getWidth();
            uint16_t CH = CPD.ComponentPtr->getHeight();
            uint16_t CL = CPD.ComponentPtr->getLength();
            uint16_t X = GridX * PLACEMENT_GRID_SIZE + PLACEMENT_GRID_SIZE / 2 - CW / 2;
            uint16_t Y = GridY * PLACEMENT_GRID_SIZE + PLACEMENT_GRID_SIZE / 2 - CH / 2;
            uint16_t Z = GridZ * PLACEMENT_GRID_SIZE + PLACEMENT_GRID_SIZE / 2 - CL / 2;
            CPD.setPlacement(X, Y, Z, Orientation::ZeroCW);
            if (!CurrentPlacement.testAndSetPlacementGrid(CPD)) {
                break;
            }
            // Loop X and Z first
            ++GridX;
            if (GridX == D.Width / PLACEMENT_GRID_SIZE) {
                GridX = 0;
                ++GridZ;
            }
            if (GridZ == D.Length / PLACEMENT_GRID_SIZE) {
                GridX = 0;
                GridZ = 0;
                ++GridY;
            }
        }
    }
    return false;
}

bool Placer::annealPlace() {
    std::cout << "Running simulated annealing placer..." << std::endl;
    const int MaxIt = 100;
    const int MaxStep = 5000;

    // Prepare swap candidates
    for (int X = 0; X != PGridW; ++X) {
        for (int Y = 0; Y != PGridH; ++Y) {
            for (int Z = 0; Z != PGridL; ++Z) {
                if (!CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z]) {
                    auto&& EmptyCPD = std::make_unique<ComponentPlacementData>(ComponentPlacementData{
                        .ComponentPtr = nullptr, .Fixed = false, .Empty = true, .GridX = X, .GridY = Y, .GridZ = Z});
                    CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z] = EmptyCPD.get();
                    CurrentPlacement.EmptyCPDs.push_back(std::move(EmptyCPD));
                    CurrentPlacement.SwapCandidate2.push_back(
                        CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z]);
                } else if (!CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z]->Fixed) {
                    CurrentPlacement.SwapCandidate1.push_back(
                        CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z]);
                    CurrentPlacement.SwapCandidate2.push_back(
                        CurrentPlacement.PGrid[X * PGridW * PGridH + Y * PGridW + Z]);
                }
            }
        }
    }

    // Init best cost
    BestCost = evalCost(CurrentPlacement);
    double InitialCost = BestCost;
    double Cost;

    int i = 0;
    while (true) {
        if (i == MaxIt) {
            std::cout << "Annealer max number of iteration reached!" << std::endl;
            break;
        }
        double Tempreture = MaxIt / (i + 1);
        for (int j = 0; j < MaxStep; ++j) {
            Cost = evalCost(CurrentPlacement);
            // TODO: Make it work for cells that takes multiple grid location
            // Generate neighbour solution
            auto [SwapFrom, SwapTo] = annealGenerateSwapNeighbour();
            annealDoSwap(SwapFrom, SwapTo);
            double NewCost = evalCost(CurrentPlacement);
            double Gain = Cost - NewCost;
            //std::cout << "Gain: " << Gain << "accept rate: " << AcceptGen(RGen) << " #:" << std::exp(Gain / Tempreture) << std::endl;
            //std::cout << "T: " << Tempreture << " Gain: " << Gain << std::endl;
            if (std::exp(Gain / Tempreture) >= AcceptGen(RGen)) {
                if (NewCost < BestCost) {
                    // Cost improved
                    applyCurrentPlacement();
                    BestCost = NewCost;
                }
            } else {
                // Undo the swap
                annealDoSwap(SwapFrom, SwapTo);
            }
        }
        std::cout << "Best cost: " << BestCost << " Initial cost: " << InitialCost << " Current cost: " << Cost
                  << std::endl;
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

double Placer::evalCost(const PlacementState& PS_) const {
    double RetVal = 0;
    for (const auto& CPD : PS_.CPDs) {
        for (auto&& [PortName, PortData] : CPD.Ports) {
            // We only calculate cost from sink input to source output
            if (PortData.Dir != Direction::Input) {
                continue;
            }
            uint16_t X = PortData.Coord.X;
            uint16_t Y = PortData.Coord.Y;
            uint16_t Z = PortData.Coord.Z;
            RetVal += std::hypot(X - PortData.ConnectedPort->Coord.X, Y - PortData.ConnectedPort->Coord.Y, Z - PortData.ConnectedPort->Coord.Z);
        }
    }
    return RetVal;
}

std::pair<int, int> Placer::annealGenerateSwapNeighbour() {
    std::uniform_int_distribution<> C1(0, CurrentPlacement.SwapCandidate1.size() - 1);
    int From = C1(RGen);
    int X1 = CurrentPlacement.SwapCandidate1[From]->GridX;
    int Y1 = CurrentPlacement.SwapCandidate1[From]->GridY;
    int Z1 = CurrentPlacement.SwapCandidate1[From]->GridZ;
    int X2, Y2, Z2;
#ifdef OPT_NEIGHBOUR
    std::vector<double> Distances(CurrentPlacement.SwapCandidate2.size());
    for (int i = 0; i < CurrentPlacement.SwapCandidate2.size(); ++i) {
        X2 = CurrentPlacement.SwapCandidate2[i]->GridX;
        Y2 = CurrentPlacement.SwapCandidate2[i]->GridY;
        Z2 = CurrentPlacement.SwapCandidate2[i]->GridZ;
        Distances[i] = std::hypot(X2 - X1, Y2 - Y1, Z2 - Z1);
    }
    std::discrete_distribution<> C2(Distances.begin(), Distances.end());
#else
    std::uniform_int_distribution<> C2(0, CurrentPlacement.SwapCandidate2.size() - 1);
#endif
    int To = C2(RGen);
    X2 = CurrentPlacement.SwapCandidate2[To]->GridX;
    Y2 = CurrentPlacement.SwapCandidate2[To]->GridY;
    Z2 = CurrentPlacement.SwapCandidate2[To]->GridZ;
    return std::make_pair(X1 * PGridW * PGridH + Y1 * PGridW + Z1, X2 * PGridW * PGridH + Y2 * PGridW + Z2);
}

void Placer::annealDoSwap(int SwapFrom_, int SwapTo_) {
    auto& From = CurrentPlacement.PGrid[SwapFrom_];
    int FromOffsetX = 0;
    int FromOffsetY = 0;
    int FromOffsetZ = 0;
    if (!From->Empty) {
        FromOffsetX = From->Place.X - From->GridX * PLACEMENT_GRID_SIZE;
        FromOffsetY = From->Place.Y - From->GridY * PLACEMENT_GRID_SIZE;
        FromOffsetZ = From->Place.Z - From->GridZ * PLACEMENT_GRID_SIZE;
    }

    auto& To = CurrentPlacement.PGrid[SwapTo_];
    int ToOffsetX = 0;
    int ToOffsetY = 0;
    int ToOffsetZ = 0;
    if (!To->Empty) {
        ToOffsetX = To->Place.X - To->GridX * PLACEMENT_GRID_SIZE;
        ToOffsetY = To->Place.Y - To->GridY * PLACEMENT_GRID_SIZE;
        ToOffsetZ = To->Place.Z - To->GridZ * PLACEMENT_GRID_SIZE;
    }

    int FromGridX = From->GridX;
    int FromGridY = From->GridY;
    int FromGridZ = From->GridZ;
    int ToGridX = To->GridX;
    int ToGridY = To->GridY;
    int ToGridZ = To->GridZ;

    if (!From->Empty && !To->Empty) {
        From->setPlacement(ToGridX * PLACEMENT_GRID_SIZE + FromOffsetX, ToGridY * PLACEMENT_GRID_SIZE + FromOffsetY,
                           ToGridZ * PLACEMENT_GRID_SIZE + FromOffsetZ, From->Place.Orient);
        To->setPlacement(FromGridX * PLACEMENT_GRID_SIZE + ToOffsetX, FromGridY * PLACEMENT_GRID_SIZE + ToOffsetY,
                         FromGridZ * PLACEMENT_GRID_SIZE + ToOffsetZ, To->Place.Orient);
    } else if (!From->Empty && To->Empty) {
        From->setPlacement(ToGridX * PLACEMENT_GRID_SIZE + FromOffsetX, ToGridY * PLACEMENT_GRID_SIZE + FromOffsetY,
                           ToGridZ * PLACEMENT_GRID_SIZE + FromOffsetZ, From->Place.Orient);
        To->setPlacement(FromGridX * PLACEMENT_GRID_SIZE, FromGridY * PLACEMENT_GRID_SIZE,
                         FromGridZ * PLACEMENT_GRID_SIZE, To->Place.Orient);
    } else if (From->Empty && !To->Empty) {
        From->setPlacement(ToGridX * PLACEMENT_GRID_SIZE, ToGridY * PLACEMENT_GRID_SIZE, ToGridZ * PLACEMENT_GRID_SIZE,
                           From->Place.Orient);
        To->setPlacement(FromGridX * PLACEMENT_GRID_SIZE + ToOffsetX, FromGridY * PLACEMENT_GRID_SIZE + ToOffsetY,
                         FromGridZ * PLACEMENT_GRID_SIZE + ToOffsetZ, To->Place.Orient);
    } else {
        std::cout << From->ComponentPtr << std::endl;
        std::cout << To->ComponentPtr << std::endl;
        throw Exception("Generated useless move.");
    }
    std::swap(CurrentPlacement.PGrid[SwapFrom_], CurrentPlacement.PGrid[SwapTo_]);
}

void Placer::ComponentPlacementData::setPlacement(uint16_t X_, uint16_t Y_, uint16_t Z_, Orientation Orient_) {
    Place.X = X_;
    Place.Y = Y_;
    Place.Z = Z_;
    Place.Orient = Orient_;
    for (auto&& [PortName, PortData] : Ports) {
        PortData.Coord = ComponentPtr->getPinLocationWithPlacement(PortName, Place);
    }
    GridX = X_ / PLACEMENT_GRID_SIZE;
    GridY = Y_ / PLACEMENT_GRID_SIZE;
    GridZ = Z_ / PLACEMENT_GRID_SIZE;
};

Placer::PlacementState::PlacementState(Design& D_) : D(D_) {
    int PGridW = D.Width / PLACEMENT_GRID_SIZE;
    int PGridH = D.Height / PLACEMENT_GRID_SIZE;
    int PGridL = D.Length / PLACEMENT_GRID_SIZE;
    PGrid.resize(PGridW * PGridH * PGridL, nullptr);

    // Create placement data structures
    const auto& Components = D.MN.getComponents();
    for (const auto& Component : Components) {
        ComponentPlacementData CPD{.Fixed = false, .Empty = false};
        CPD.ComponentPtr = Component.get();
        for (const auto& PinName : Component->getPinNames()) {
            CPD.Ports.emplace(PinName, Port{.Name = PinName, .Dir = CPD.ComponentPtr->getPinDir(PinName)});
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
}

bool Placer::PlacementState::testAndSetPlacementGrid(ComponentPlacementData& CPD_) {
    const auto& ComponentRange = CPD_.ComponentPtr->getRangeWithPlacement(CPD_.Place);
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    if (P1.X >= D.Width || P1.Y >= D.Height || P1.Z >= D.Length) {
        return true;
    }
    if (P2.X > D.Width || P2.Y > D.Height || P2.Z > D.Length) {
        return true;
    }
    int GridX1 = P1.X / PLACEMENT_GRID_SIZE;
    int GridY1 = P1.Y / PLACEMENT_GRID_SIZE;
    int GridZ1 = P1.Z / PLACEMENT_GRID_SIZE;
    int GridX2 = (P2.X - 1) / PLACEMENT_GRID_SIZE + 1;
    int GridY2 = (P2.Y - 1) / PLACEMENT_GRID_SIZE + 1;
    int GridZ2 = (P2.Z - 1) / PLACEMENT_GRID_SIZE + 1;

    for (int X = GridX1; X != GridX2; ++X) {
        for (int Y = GridY1; Y != GridY2; ++Y) {
            for (int Z = GridZ1; Z != GridZ2; ++Z) {
                if (PGrid[X * (D.Width / PLACEMENT_GRID_SIZE) * (D.Height / PLACEMENT_GRID_SIZE) +
                          Y * (D.Width / PLACEMENT_GRID_SIZE) + Z]) {
                    return true;
                } else {
                    PGrid[X * (D.Width / PLACEMENT_GRID_SIZE) * (D.Height / PLACEMENT_GRID_SIZE) +
                          Y * (D.Width / PLACEMENT_GRID_SIZE) + Z] = &CPD_;
                }
            }
        }
    }
    return false;
}

bool Placer::PlacementState::setPlacementGrid(ComponentPlacementData& CPD_) {
    const auto& ComponentRange = CPD_.ComponentPtr->getRangeWithPlacement(CPD_.Place);
    const auto& P1 = ComponentRange.first;
    const auto& P2 = ComponentRange.second;
    if (P1.X >= D.Width || P1.Y >= D.Height || P1.Z >= D.Length) {
        return true;
    }
    if (P2.X > D.Width || P2.Y > D.Height || P2.Z > D.Length) {
        return true;
    }
    int GridX1 = P1.X / PLACEMENT_GRID_SIZE;
    int GridY1 = P1.Y / PLACEMENT_GRID_SIZE;
    int GridZ1 = P1.Z / PLACEMENT_GRID_SIZE;
    int GridX2 = (P2.X - 1) / PLACEMENT_GRID_SIZE + 1;
    int GridY2 = (P2.Y - 1) / PLACEMENT_GRID_SIZE + 1;
    int GridZ2 = (P2.Z - 1) / PLACEMENT_GRID_SIZE + 1;

    for (int X = GridX1; X != GridX2; ++X) {
        for (int Y = GridY1; Y != GridY2; ++Y) {
            for (int Z = GridZ1; Z != GridZ2; ++Z) {
                PGrid[X * (D.Width / PLACEMENT_GRID_SIZE) * (D.Height / PLACEMENT_GRID_SIZE) +
                      Y * (D.Width / PLACEMENT_GRID_SIZE) + Z] = &CPD_;
            }
        }
    }
    return false;
}
