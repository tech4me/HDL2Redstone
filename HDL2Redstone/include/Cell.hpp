#include <Schematic.hpp>

class Cell {
    enum class Type {
        Nand
    };

    Cell() = delete;

    Cell(Type T);

    private:
        Schematic Sch;
        // Here we should also store timing, volume, and constraint info
};
