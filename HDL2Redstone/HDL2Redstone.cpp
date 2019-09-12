#include <iostream>

#include <CellLibrary.hpp>
#include <ModuleNetlist.hpp>
#include <Schematic.hpp>

using namespace HDL2Redstone;

int main(int argc, char* argv[]) {
    /*
    Schematic S;
    if (S.loadSchematic("../HDL2Redstone/cell_lib/BlankShape.schematic")) {
        std::cout << "failed to load schematic" << std::endl;
    }
    */
    if (argc > 1) {
        std::string CellLibJSON(argv[1]);
        CellLibrary CL(CellLibJSON);
        ModuleNetlist MN("design.blif", CL);
        std::cout << MN;
    } else {
        std::cout << "ERROR" << std::endl;
    }

    return 0;
}
