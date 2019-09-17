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
            // ModuleNetlist MN("design.blif", CL);
            Design D(100, 200, 300, "design.blif", CL); // take 100x200x300 as an example
            std::cout << D;
            std::cout << "Now Placing ...\n";
            D.place();
            std::cout << D;
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
