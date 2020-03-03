#pragma once

#include <iostream>
#include <map>
#include <stack>
#include <vector>

#include "Design.hpp"

namespace HDL2Redstone {
class Timing {
  public:
    Timing(Design& D_);
    //~Timing();

    const std::map<Component*, std::vector<std::pair<Component*, int>>>& getTG() { return TG; };
    void setTG(Component* Key_, const std::vector<std::pair<Component*, int>>& Val_) {
        TG.at(Key_).insert(TG.at(Key_).end(), Val_.begin(), Val_.end());
    };

    double computePropDelay();

  private:
    void topoHelper(Component* k_, std::map<Component*, bool>& Visited_);
    void topoSort();
    // Timing graph, key=components, value=list of components connected to key, with their cost
    std::map<Component*, std::vector<std::pair<Component*, int>>> TG;
    // topologically sorted graph
    std::stack<Component*> SortedTG;
    friend std::ostream& operator<<(std::ostream& out, const Timing& T_);
};
} // namespace HDL2Redstone
