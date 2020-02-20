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
            DesignConstraint DC;

            if (DC.readDCFile("design_constr.json")) {
                return 1;
            }

            // and gate
            // DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("b", {4, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("c", {1, 0, 19, Orientation::ZeroCW});

            // 4to1 mux
            //    DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("b", {3, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("c", {6, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("d", {9, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("x", {12, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("y", {15, 0, 0, Orientation::ZeroCW});
            //    DC.addForcedPlacement("dout", {10, 0, 49, Orientation::ZeroCW});

            // 2to1 mux
            //  DC.addForcedPlacement("a", {0, 0, 0, Orientation::ZeroCW});
            //  DC.addForcedPlacement("b", {20, 0, 0, Orientation::ZeroCW});
            //  DC.addForcedPlacement("x", {40, 0, 0, Orientation::ZeroCW});
            //  DC.addForcedPlacement("dout", {30, 0, 49, Orientation::ZeroCW});

            // 4 bit adder
            // DC.addForcedPlacement("a[0]", {0, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("a[1]", {10, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("a[2]", {20, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("a[3]", {30, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("b[0]", {70, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("b[1]", {80, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("b[2]", {90, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("b[3]", {99, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("cin", {50, 0, 0, Orientation::ZeroCW});
            // DC.addForcedPlacement("sout[0]", {0, 0, 99, Orientation::ZeroCW});
            // DC.addForcedPlacement("sout[1]", {20, 0, 99, Orientation::ZeroCW});
            // DC.addForcedPlacement("sout[2]", {40, 0, 99, Orientation::ZeroCW});
            // DC.addForcedPlacement("sout[3]", {60, 0, 99, Orientation::ZeroCW});
            // DC.addForcedPlacement("sout[4]", {80, 0, 99, Orientation::ZeroCW});
	    
	    // 4 bit multiplier
	    /*DC.addForcedPlacement("a[0]", {0, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("a[1]", {10, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("a[2]", {20, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("a[3]", {30, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("b[0]", {70, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("b[1]", {80, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("b[2]", {90, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("b[3]", {99, 0, 0, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[0]", {0, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[1]", {20, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[2]", {40, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[3]", {60, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[4]", {80, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[5]", {100, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[6]", {120, 0, 149, Orientation::ZeroCW});
	    DC.addForcedPlacement("result[7]", {140, 0, 149, Orientation::ZeroCW});*/

            Design D("design.blif", CL, DC);
            // std::cout << D;
	                
	    std::cout << "Now Running Place & Route..." << std::endl;
            if (D.doPlaceAndRoute()) {
                std::cout << "Place/Route failed!!!" << std::endl;
                return 1;
            }
            // std::cout << D;
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
