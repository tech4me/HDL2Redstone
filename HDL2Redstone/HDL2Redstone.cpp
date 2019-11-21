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
            /*
            DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("b", {4, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("c", {1, 0, 19, Orientation::ZeroCW});
            */
            /*
            DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("b", {3, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("c", {6, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("d", {9, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("x", {12, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("y", {15, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("dout", {10, 0, 49, Orientation::ZeroCW});
            */
            DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("b", {20, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("x", {40, 0, 0, Orientation::ZeroCW});
            DC.addForcedPlacement("dout", {30, 0, 49, Orientation::ZeroCW});

            Design D(50, 20, 50, "design.blif", CL, DC);
            std::cout << D;
            std::cout << "Now Running Place & Route..." << std::endl;
            if (!D.doPlaceAndRoute()) {
                std::cout << "Place/Route failed!!!" << std::endl;
                return 1;
            }
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
