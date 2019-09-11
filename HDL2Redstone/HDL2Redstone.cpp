#include <iostream>

#include <CellLibrary.hpp>
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
    CellLibrary CL("/home/kitty/496/HDL2Redstone/HDL2Redstone/cell_lib/HDL2Redstone.json");
    ModuleNetlist MN("design.blif", CL);
    std::cout << MN;
    return 0;
}
