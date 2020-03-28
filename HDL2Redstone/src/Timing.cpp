#include <Exception.hpp>
#include <Timing.hpp>
#include <limits>

using namespace HDL2Redstone;

void Timing::RGTreeHelper(Component* Src, Component* Curr, Path& P_) {
    if (TG.find(Curr) == TG.end() || TG.at(Curr).empty())
        return;

    for (const auto& Node : TG[Curr]) {
        // add info of this sink to the path
        Path P = P_;
        P.CombPath.emplace_back(Node.ConnPtr, Node.CompPtr);
        P.Delay += Node.Delay;

        if (Node.CompPtr->getType() == "DFF") {
            std::pair<Component*, Component*> Key(Src, Node.CompPtr);

            if (RG.find(Key) != RG.end()) {
                RG.at(Key).push_back(P);
            } else {
                std::vector<Path> temp;
                temp.push_back(P);
                RG.emplace(Key, temp);
            }
        } else {
            RGTreeHelper(Src, Node.CompPtr, P);
        }
    }
}

Timing::Timing(Design& D_) { buildTG(D_); }

void Timing::buildTG(Design& D_) {
    TG.clear();
    RG.clear();
    ClkTree.clear();

    for (int j = 0; j < D_.getModuleNetlist().getConnections().size(); j++) {
        D_.getModuleNetlist().getConnections()[j]->calculateDelay();

        const auto& Sinks = D_.getModuleNetlist().getConnections()[j]->getSinkPortConnections();
        std::vector<TGNode> Temp;
        // grouping sink with their delays together
        for (int i = 0; i < Sinks.size(); i++) {
            Temp.emplace_back(Sinks[i].first, D_.getModuleNetlist().getConnections()[j].get(),
                              D_.getModuleNetlist().getConnections()[j]->getSinkDelays()[i]);
        }

        // find clk tree
        if (D_.getModuleNetlist().getConnections()[j]->getName().find("clk") != std::string::npos) {
            /*ClkTree.first = D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first;
            ClkTree.second = Temp;*/
            ClkSrc = D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first;
            for (const auto& Node : Temp) {
                ClkTree.emplace(Node.CompPtr, Node.Delay);
            }
        } else { // regular connection
            // insert into timing graph, using source as starting node
            auto it = TG.find(D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first);
            if (it != TG.end()) { // update what's connected to this node
                setTG(it->first, Temp);
            } else { // insert into map
                TG.emplace(D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first, Temp);
            }
        }
    }

    // build register graph
    for (const auto& [k, v] : TG) {
        if (k->getType() == "DFF") {
            Path P;
            RGTreeHelper(k, k, P);
        }
    }
}

void Timing::topoHelper(Component* k_, std::map<Component*, bool>& Visited_) {
    Visited_[k_] = true;

    for (auto Node : TG[k_]) {
        if (!Visited_[Node.CompPtr])
            topoHelper(Node.CompPtr, Visited_);
    }

    SortedTG.push(k_);
}

void Timing::topoSort() {
    std::map<Component*, bool> Visited;
    for (const auto& [k, v] : TG) {
        Visited.emplace(k, false);
    }

    for (const auto& [k, v] : TG) {
        if (!Visited[k])
            topoHelper(k, Visited);
    }
}

int Timing::computePropDelay() {
    topoSort();
    auto SortedTG_ = SortedTG;
    // consider putting everything below in a func?

    // longest dist to each vertex
    std::map<Component*, int> dist;
    for (const auto& [k, v] : TG) {
        dist[k] = 0;
    }

    while (!SortedTG_.empty()) {
        auto curr = SortedTG_.top(); // curr is a component ptr
        SortedTG_.pop();

        for (const auto& Node : TG.at(curr)) {
            int temp_dist = dist.at(curr) + Node.Delay;
            // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
            if (dist.at(Node.CompPtr) < temp_dist) {
                dist.at(Node.CompPtr) = temp_dist;
                // save actual path here if needed
                // std::cout<<"dist "<<v.first<<" "<<dist.at(v.first)<<std::endl;
            }
        }
    }

    int PropDelay = -1;
    for (const auto& [k, v] : dist) {
        // DOUT(<< "comp " << k << " dist:" << v << std::endl);
        if (v > PropDelay) {
            PropDelay = v;
        }
        // for testing shortest and longest path
        // if (v == 5)
        //  dest = k;
        // if (v == 0 && src==NULL)
        //  src = k;
    }

    return PropDelay;
}

void Timing::findHoldViolations() {
    for (auto& [K, V] : RG) {
        // Tskew calculated as delay at sink - delay at src
        int Tskew = ClkTree.at(K.second) - ClkTree.at(K.first);
        int Tcomb_min = Thold + Tskew - Tcq;

        for (auto& P : V) {
            if (P.Delay < Tcomb_min) {
                P.DelayNeeded = Tcomb_min - P.Delay;
                HoldViolatedPaths.push_back(P);
            }
        }
    }
}

double Timing::computeFmax(Design& D_) {
    if (RG.empty()) {
        return 0;
    }
    buildTG(D_);
    int Tmin = -1; // min clk period needed
    for (const auto& [K, V] : RG) {
        // Tskew calculated as delay at sink - delay at src
        int Tskew = ClkTree.at(K.second) - ClkTree.at(K.first);
        for (const auto& P : V) {
            int Tclk = Tcq + P.Delay + Tsu - Tskew;
            if (Tclk > Tmin) {
                Tmin = Tclk;
            }
        }
    }
    return 1.0 / Tmin;
}

std::vector<Component*> Timing::findShortestDelay(Component* src, Component* dest) {
    auto SortedTG_ = SortedTG;
    std::vector<Component*> Result; // TODO: should it be stack here, so no need to reverse order after done

    // longest dist to each vertex; TODO: Maybe change to member var
    std::map<Component*, int> dist;
    // contains the longest path to each vertex
    std::map<Component*, Component*> Path;
    for (const auto& [k, v] : TG) {
        dist[k] = std::numeric_limits<int>::infinity();
        Path[k] = NULL;
    }
    dist[src] = 0;
    // std::cout<<"src"<<src<<" dest:"<<dest<<std::endl;
    // Path[src] = src;

    while (!SortedTG_.empty()) {
        auto curr = SortedTG_.top(); // curr is a component ptr
        // std::cout<<"now doing "<<curr<<std::endl;
        SortedTG_.pop();

        if (dist[curr] != std::numeric_limits<int>::infinity()) {
            for (const auto& Node : TG.at(curr)) {
                int temp_dist = dist.at(curr) + Node.Delay;
                // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
                if (dist.at(Node.CompPtr) > temp_dist) {
                    dist.at(Node.CompPtr) = temp_dist;
                    Path.at(Node.CompPtr) = curr;
                    // std::cout<<"dist "<<v.first<<" "<<dist.at(v.first)<<std::endl;
                }
            }
        }
    }

    Result.push_back(dest);
    Component* Pred = Path.at(dest);
    while (Pred != src) {
        Result.push_back(Pred);
        Pred = Path.at(Pred);
    }
    Result.push_back(src);

    // return dist[dest];
    return Result;
}

std::vector<Component*> Timing::findLongestDelay(Component* src, Component* dest) {
    auto SortedTG_ = SortedTG;
    std::vector<Component*> Result; // TODO: should it be stack here, so no need to reverse order after done

    // longest dist to each vertex; TODO: Maybe change to member var
    std::map<Component*, int> dist;
    // contains the longest path to each vertex
    std::map<Component*, Component*> Path;
    for (const auto& [k, v] : TG) {
        dist[k] = -1;
        Path[k] = NULL;
    }
    dist[src] = 0;
    // std::cout<<"src"<<src<<" dest:"<<dest<<std::endl;
    // Path[src] = src;

    while (!SortedTG_.empty()) {
        auto curr = SortedTG_.top(); // curr is a component ptr
        // std::cout<<"now doing "<<curr<<std::endl;
        SortedTG_.pop();

        if (dist[curr] != -1) {
            for (const auto& Node : TG.at(curr)) {
                int temp_dist = dist.at(curr) + Node.Delay;
                // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
                if (dist.at(Node.CompPtr) < temp_dist) {
                    dist.at(Node.CompPtr) = temp_dist;
                    Path.at(Node.CompPtr) = curr;
                    // std::cout<<"dist "<<v.first<<" "<<dist.at(v.first)<<std::endl;
                }
            }
        }
    }

    Result.push_back(dest);
    Component* Pred = Path.at(dest);
    while (Pred != src) {
        Result.push_back(Pred);
        Pred = Path.at(Pred);
    }
    Result.push_back(src);

    // return dist[dest];
    return Result;
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Timing& T_) {
    auto TG = T_.TG;
    out << "Timing Graph:" << std::endl;
    for (const auto& [k, v] : TG) {
        out << "    Component " << k->getType() << " " << k << ": ";
        for (const auto& sink : v) {
            out << "(" << sink.CompPtr << ", " << sink.ConnPtr << ", " << sink.Delay << "), ";
        }
        out << std::endl;
    }

    auto STG = T_.SortedTG;
    out << "Topo sorted graph:" << std::endl;
    out << "    ";
    while (!STG.empty()) {
        out << "Component " << STG.top()->getType() << " " << STG.top() << "-> ";
        STG.pop();
    }
    out << std::endl;

    auto RG = T_.RG;
    out << "Register Graph:" << std::endl;
    for (const auto& [k, v] : RG) {
        out << "  Path from Reg " << k.first << "-> " << k.second << ":" << std::endl;
        for (const auto& path : v) {
            for (const auto& comp : path.CombPath) {
                out << "    (" << comp.second << " " << comp.first << ") -> ";
            }
            out << std::endl;
            out << "  Tcomb:" << path.Delay << std::endl;
        }
    }

    auto CT = T_.ClkTree;
    out << "Clock Tree:" << std::endl;
    out << "  src:" << T_.ClkSrc << std::endl;
    for (const auto& [k, v] : CT) {
        out << "  reg:" << k << " delay:" << v << std::endl;
    }

    out << std::endl;
    return out;
}
} // namespace HDL2Redstone
