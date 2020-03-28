#include <chrono>
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
            auto StartTime = std::chrono::system_clock::now();
            std::string CellLibDir(argv[2]);
            std::cout << "Loading cell library..." << std::endl;
            CellLibrary CL(CellLibDir);
            std::cout << "Loading design constraint..." << std::endl;
            DesignConstraint DC;

            if (DC.readDCFile("design_constr.json")) {
                return 1;
            }
            std::cout << "Loading BLIF netlist..." << std::endl;
            Design D("design.blif", CL, DC, std::stoi(argv[1]));
            if (D.doPlaceAndRoute()) {
                std::cout << "Place & Route failed!!!" << std::endl;
                return 1;
            }
            std::cout << "Place & Route success!" << std::endl;
            std::cout << "Exporting schematic..." << std::endl;
            D.exportDesign().exportSchematic("design.schem");
            std::chrono::duration<double> Time = std::chrono::system_clock::now() - StartTime;
            std::cout << "Design compilation finished! Total time: " << Time.count() << "s" << std::endl;
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
