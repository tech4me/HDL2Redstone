#include <iostream>

#include <CellLibrary.hpp>
#include <Design.hpp>
#include <Exception.hpp>
#include <ModuleNetlist.hpp>
#include <Schematic.hpp>

using namespace HDL2Redstone;

int main(int argc, char* argv[]) {
    try {
        if (argc > 1) {
            std::string CellLibDir(argv[1]);
            CellLibrary CL(CellLibDir);
            std::cout << CL;
            DesignConstraint DC;
            DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("b", {2, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("c", {1, 0, 9, Orientation::ZeroCW});
            Design D(10, 10, 10, "design.blif", CL, DC);
            std::cout << D;
            std::cout << "Now Placing ..." << std::endl;
            D.doPlaceAndRoute();
            std::cout << D;
            D.exportDesign().exportSchematic("design.schem");
        } else {
            throw Exception("Incorrect number of arguments provided.");
        }
    } catch (Exception& E) {
        std::cout << "Error: " << E.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Error: Unknown Exception occurred!" << std::endl;
        return 1;
    }
    return 0;
}
