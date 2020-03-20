#include <Exception.hpp>
#include <Timing.hpp>
#include <limits>

using namespace HDL2Redstone;

Timing::Timing(Design& D_) {
    for (int j = 0; j < D_.getModuleNetlist().getConnections().size(); j++) {
        D_.getModuleNetlist().getConnections()[j]->calculateDelay();

        const auto& Sinks = D_.getModuleNetlist().getConnections()[j]->getSinkPortConnections();
        std::vector<std::pair<Component*, int>> Temp;
        // grouping sink with their delays together
        for (int i = 0; i < Sinks.size(); i++) {
            Temp.emplace_back(Sinks[i].first, D_.getModuleNetlist().getConnections()[j]->getSinkDelays()[i]);
        }
        // insert into timing graph, using source as starting node
        auto it = TG.find(D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first);
        if (it != TG.end()) { // update what's connected to this node
            setTG(it->first, Temp);
        } else { // insert into map
            TG.emplace(D_.getModuleNetlist().getConnections()[j]->getSourcePortConnection().first, Temp);
        }
    }
}

void Timing::topoHelper(Component* k_, std::map<Component*, bool>& Visited_) {
    Visited_[k_] = true;

    for (auto pair : TG[k_]) {
        if (!Visited_[pair.first])
            topoHelper(pair.first, Visited_);
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

double Timing::computePropDelay() {
    topoSort();
    auto SortedTG_ = SortedTG;
    // if given a src and a dest, initialize everything to -1, src to 0; then only check those != -1
    // in the end, just check dist[dest] for longest path from src to dest
    // consider putting everything below in a func?
    // make a copy of SortedTG!!!???

    // longest dist to each vertex; TODO: Maybe change to member var and make it contain actual longest path
    std::map<Component*, double> dist;
    for (const auto& [k, v] : TG) {
        // dist[k] = -1;
        dist[k] = 0; // TODO: Should this be initialized to 0 or -1? if -1, then need to initialize all roots to 0,
                     // which requires reverse graph?
    }
    // dist[SortedTG.top()] = 0;

    while (!SortedTG_.empty()) {
        auto curr = SortedTG_.top(); // curr is a component ptr
        // std::cout<<"now doing "<<curr<<std::endl;
        SortedTG_.pop();

        //	if (dist[curr] != -1) {
        for (const auto& v : TG.at(curr)) {
            double temp_dist = dist.at(curr) + v.second;
            // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
            if (dist.at(v.first) < temp_dist) {
                dist.at(v.first) = temp_dist;
                // TODO: Prob save actual path here
                // std::cout<<"dist "<<v.first<<" "<<dist.at(v.first)<<std::endl;
            }
        }
        //	}
    }

    double PropDelay = -1;
    for (const auto& [k, v] : dist) {
        DOUT(<< "comp " << k << " dist:" << v << std::endl);
        if (v > PropDelay) {
            PropDelay = v;
        }
        // for testing shortest and longest path
        /*if (v == 5)
            dest = k;
        if (v == 0 && src==NULL)
            src = k;*/
    }

    return PropDelay;
}

std::vector<Component*> Timing::findShortestDelay(Component* src, Component* dest) {
    auto SortedTG_ = SortedTG;
    std::vector<Component*> Result; // TODO: should it be stack here, so no need to reverse order after done

    // longest dist to each vertex; TODO: Maybe change to member var and make it contain actual path
    std::map<Component*, double> dist;
    // contains the longest path to each vertex
    std::map<Component*, Component*> Path;
    for (const auto& [k, v] : TG) {
        dist[k] = std::numeric_limits<double>::infinity();
        Path[k] = NULL;
    }
    dist[src] = 0;
    // std::cout<<"src"<<src<<" dest:"<<dest<<std::endl;
    // Path[src] = src;

    while (!SortedTG_.empty()) {
        auto curr = SortedTG_.top(); // curr is a component ptr
        // std::cout<<"now doing "<<curr<<std::endl;
        SortedTG_.pop();

        if (dist[curr] != std::numeric_limits<double>::infinity()) {
            for (const auto& v : TG.at(curr)) {
                double temp_dist = dist.at(curr) + v.second;
                // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
                if (dist.at(v.first) > temp_dist) {
                    dist.at(v.first) = temp_dist;
                    Path.at(v.first) = curr;
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

    // longest dist to each vertex; TODO: Maybe change to member var and make it contain actual path
    std::map<Component*, double> dist;
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
            for (const auto& v : TG.at(curr)) {
                double temp_dist = dist.at(curr) + v.second;
                // std::cout<<"dist at u="<<dist.at(curr)<<" w(u,v)="<<v.second<<std::endl;
                if (dist.at(v.first) < temp_dist) {
                    dist.at(v.first) = temp_dist;
                    Path.at(v.first) = curr;
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
            out << "(" << sink.first << ", " << sink.second << "), ";
        }
        out << std::endl;
    }

    auto STG = T_.SortedTG;
    out << "Topo sorted graph:" << std::endl;
    while (!STG.empty()) {
        out << "Component " << STG.top()->getType() << " " << STG.top() << "-> ";
        STG.pop();
    }
    out << std::endl;
    return out;
}
} // namespace HDL2Redstone
