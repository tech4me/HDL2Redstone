#include <algorithm>
#include <cmath>
#include <limits>
#include <sstream>

#include <Exception.hpp>
#include <Placer.hpp>

#define PLACEMENT_GRID_SIZE 10
#define MAX_INITIAL_TEMPERATURE_MOVE 10000

using namespace HDL2Redstone;

Placer::Placer(Design& D_) : D(D_), CurrentPlacement(D_), RGen(D.Seed ? D.Seed : RD()), AcceptGen(0.0, 1.0) {
    BestCost = std::numeric_limits<int>::max();
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
    const int MaxIt = 1000;
    const int MaxStep = 5000;

    // Prepare swap candidates
    for (int X = 0; X != PGridW; ++X) {
        for (int Y = 0; Y != PGridH; ++Y) {
            for (int Z = 0; Z != PGridL; ++Z) {
                if (!CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z]) {
                    auto&& EmptyCPD = std::make_unique<ComponentPlacementData>(ComponentPlacementData{
                        .ComponentPtr = nullptr, .Fixed = false, .Empty = true, .GridX = X, .GridY = Y, .GridZ = Z});
                    CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z] = EmptyCPD.get();
                    //std::cout << "SET " << X << " " << Y << " " << Z << " " << EmptyCPD.get() << std::endl;
                    CurrentPlacement.EmptyCPDs.push_back(std::move(EmptyCPD));
                    CurrentPlacement.SwapCandidate2.push_back(
                        CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z]);
                } else if (!CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z]->Fixed) {
                    //std::cout << X << " " << Y << " " << Z << " " << CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z] << std::endl;
                    CurrentPlacement.SwapCandidate1.push_back(
                        CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z]);
                    CurrentPlacement.SwapCandidate2.push_back(
                        CurrentPlacement.PGrid[X * PGridH * PGridL + Y * PGridL + Z]);
                }
            }
        }
    }

    double Temperature = calculateInitTemperature();
    // Init best cost
    BestCost = evalCost(CurrentPlacement);
    int InitialCost = BestCost;
    int Cost;
    std::cout << "Annealer initial cost: " << InitialCost << std::endl;

    int i = 0;
    while (true) {
        if (Temperature < 0.1) {
            std::cout << "Annealer cold temperature reached!" << std::endl;
            break;
        } else if (i == MaxIt) {
            std::cout << "Annealer max number of iteration reached!" << std::endl;
            break;
        }
        for (int j = 0; j < MaxStep; ++j) {
            Cost = evalCost(CurrentPlacement);
            // TODO: Make it work for cells that takes multiple grid location
            // Generate neighbour solution
            auto [SwapFrom, SwapTo] = annealGenerateSwapNeighbour();
            annealDoSwap(SwapFrom, SwapTo);
            int NewCost = evalCost(CurrentPlacement);
            int Gain = Cost - NewCost;
            if (std::exp(Gain / Temperature) >= AcceptGen(RGen)) {
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
        std::cout << "Best cost: " << BestCost << " Iteration: " << i << " Temperature: " << Temperature << std::endl;
        Temperature *= 0.95;
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
        const auto& [P1, P2] = Component->getRange();
        if (P1.X >= D.Width || P1.Y >= D.Height || P1.Z >= D.Length) {
            return true;
        }
        if (P2.X > D.Width || P2.Y > D.Height || P2.Z > D.Length) {
            return true;
        }
        for (auto X = P1.X; X != P2.X; ++X) {
            for (auto Y = P1.Y; Y != P2.Y; ++Y) {
                for (auto Z = P1.Z; Z != P2.Z; ++Z) {
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

int Placer::evalCost(const PlacementState& PS_) const {
    int RetVal = 0;
    for (const auto& CPD : PS_.CPDs) {
        for (auto&& [PortName, PortData] : CPD.Ports) {
            if (PortData.Dir == Direction::Output) {
#ifndef USE_HPWL
                // HPWL method
                uint16_t XMax = PortData.Coord.X;
                uint16_t XMin = PortData.Coord.X;
                uint16_t YMax = PortData.Coord.Y;
                uint16_t YMin = PortData.Coord.Y;
                uint16_t ZMax = PortData.Coord.Z;
                uint16_t ZMin = PortData.Coord.Z;
                if (PortData.Dir == Direction::Output) {
                    uint16_t X2, Y2, Z2;
                    for (const auto& P : PortData.ConnectedPorts) {
                        XMax = std::max(XMax, P->Coord.X);
                        XMin = std::min(XMin, P->Coord.X);
                        YMax = std::max(YMax, P->Coord.Y);
                        YMin = std::min(YMin, P->Coord.Y);
                        ZMax = std::max(ZMax, P->Coord.Z);
                        ZMin = std::min(ZMin, P->Coord.Z);
                    }
                    RetVal += XMax - XMin + YMax - YMin + ZMax - ZMin;
                }
#else
                uint16_t X = PortData.Coord.X;
                uint16_t Y = PortData.Coord.Y;
                uint16_t Z = PortData.Coord.Z;
                for (const auto& P : PortData.ConnectedPorts) {
                    RetVal += std::abs(X - P->Coord.X) + std::abs(Y - P->Coord.Y) + std::abs(Z - P->Coord.Z);
                }
#endif
            }
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
    return std::make_pair(X1 * PGridH * PGridL + Y1 * PGridL + Z1, X2 * PGridH * PGridL + Y2 * PGridL + Z2);
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

double Placer::calculateInitTemperature() {
    int DeltaE = std::numeric_limits<int>::min();
    int CurrentCost = evalCost(CurrentPlacement);
    for (int i = 0; i < MAX_INITIAL_TEMPERATURE_MOVE; ++i) {
        auto [SwapFrom, SwapTo] = annealGenerateSwapNeighbour();
        annealDoSwap(SwapFrom, SwapTo);
        int NewCost = evalCost(CurrentPlacement);
        int TempDeltaE = std::abs(NewCost - CurrentCost);
        if (TempDeltaE > DeltaE) {
            DeltaE = TempDeltaE;
        }
        // Undo changes
        annealDoSwap(SwapFrom, SwapTo);
    }
    return DeltaE;
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
                    // Only add info at the source side
                    It1->Ports.at(SourcePort.second).ConnectedPorts.push_back(&(It2->Ports.at(SinkPort.second)));
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
                if (PGrid[X * (D.Height / PLACEMENT_GRID_SIZE) * (D.Length / PLACEMENT_GRID_SIZE) +
                          Y * (D.Length / PLACEMENT_GRID_SIZE) + Z]) {
                    return true;
                } else {
                    PGrid[X * (D.Height / PLACEMENT_GRID_SIZE) * (D.Length / PLACEMENT_GRID_SIZE) +
                          Y * (D.Length / PLACEMENT_GRID_SIZE) + Z] = &CPD_;
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
                PGrid[X * (D.Height / PLACEMENT_GRID_SIZE) * (D.Length / PLACEMENT_GRID_SIZE) +
                      Y * (D.Length / PLACEMENT_GRID_SIZE) + Z] = &CPD_;
            }
        }
    }
    return false;
}
