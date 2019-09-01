#include <Schematic.hpp>
#include <iostream>

int main() {
    Schematic S;
    if (S.loadSchematic("../HDL2Redstone/cell_lib/BlankShape.schematic")) {
        std::cout << "failed to load schematic" << std::endl;
    }
    return 0;
}
