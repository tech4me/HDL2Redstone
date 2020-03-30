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
    void buildTG(Design& D_);
    //~Timing();

    struct TGNode {
        Component* CompPtr;
        Connection* ConnPtr;
        int Delay;
        TGNode(Component* CompPtr_, Connection* ConnPtr_, int Delay_)
            : CompPtr(CompPtr_), ConnPtr(ConnPtr_), Delay(Delay_) {}
    }; // timing graph node

    struct Path {
        std::vector<std::pair<Connection*, Component*>> CombPath;
        int Delay;
        int DelayNeeded; // to fix hold time violation
        Path() {
            Delay = 0;
            DelayNeeded = 0;
        }
    }; // Path b/t 2 reg and its comb delay

    const std::map<Component*, std::vector<TGNode>>& getTG() { return TG; };
    void setTG(Component* Key_, const std::vector<TGNode>& Val_) {
        TG.at(Key_).insert(TG.at(Key_).end(), Val_.begin(), Val_.end());
    };

    int computePropDelay();
    double computeFmax(Design& D_);
    // return longest path from src to dest, note result is in reverse order (start with dest)
    std::vector<Component*> findLongestDelay(Component* src, Component* dest);
    std::vector<Component*> findShortestDelay(Component* src, Component* dest);

    void findHoldViolations();
    std::vector<Path> HoldViolatedPaths; // for router retiming, contains all Path violating hold time constraint

    Component *src = NULL, *dest; // for testing

  private:
    void topoHelper(Component* k_, std::map<Component*, bool>& Visited_);
    void topoSort();
    // Timing graph, key=components, value=list of components connected to key, with their cost
    std::map<Component*, std::vector<TGNode>> TG;
    // topologically sorted graph
    std::stack<Component*> SortedTG;

    // clock tree, first=input clk src, second=all the registers with their clk delays (amt of delay b/t input clk &
    // their input clk)
    // std::pair<Component*, std::vector<TGNode>> ClkTree;
    Component* ClkSrc; // not sure if this is ever needed
    // clock tree, key=register, value=delay from clk src to reg clk
    std::map<Component*, int> ClkTree;

    void RGTreeHelper(Component* Src, Component* Curr, Path& P_);
    // register graph, key=src+dest reg, value=list of Path between the 2 reg
    std::map<std::pair<Component*, Component*>, std::vector<Path>> RG;

    // TODO: To be read from JSON
    int Tcq = 3;
    int Thold = 0;
    int Tsu = 1;

    friend std::ostream& operator<<(std::ostream& out, const Timing& T_);
};
} // namespace HDL2Redstone
