#include <iostream>

#include <ModuleNetlist.hpp>
#include <Schematic.hpp>

using namespace HDL2Redstone;

int main() {
    /*
    Schematic S;
    if (S.loadSchematic("../HDL2Redstone/cell_lib/BlankShape.schematic")) {
        std::cout << "failed to load schematic" << std::endl;
    }
    */
    ModuleNetlist MN("design.blif");
    std::cout << MN;
    return 0;
}
