#include <fstream>
#include <io/izlibstream.h>
#include <io/stream_reader.h>
#include <iostream>
#include <nbt_tags.h>
#include <sstream>
#include <string>

#include "Schematic.hpp"

// This comes from https://minecraft.gamepedia.com/Schematic_file_format
static constexpr auto SCH_WIDTH = "Width";
static constexpr auto SCH_HEIGHT = "Height";
static constexpr auto SCH_LENGTH = "Length";
static constexpr auto SCH_MATERIALS = "Materials";
static constexpr auto SCH_BLOCKS = "Blocks";
static constexpr auto SCH_DATA = "Data";
static constexpr auto SCH_ENTITIES = "Entities";
static constexpr auto SCH_TILE_ENTITIES = "TileEntities";

bool Schematic::loadSchematic(const std::string& File) {
    std::ifstream FS(File, std::ios::binary);
    if (FS.fail()) {
        std::cerr << "Error: Problem loading File: " << File << std::endl;
        return true;
    }

    zlib::izlibstream ZS(FS);
    auto P = nbt::io::read_compound(ZS);
    if (P.first != "Schematic") {
        std::cerr << "Error: Is " << File << " a schematic file?" << std::endl;
        return true;
    }
    auto C = *P.second;

    try {
        Width = C.at(SCH_WIDTH).as<nbt::tag_short>();
        Height = C.at(SCH_HEIGHT).as<nbt::tag_short>();
        Length = C.at(SCH_LENGTH).as<nbt::tag_short>();
        Materials = C.at(SCH_MATERIALS).as<nbt::tag_string>();
        Blocks = C.at(SCH_BLOCKS).as<nbt::tag_byte_array>().get();
        Data = C.at(SCH_DATA).as<nbt::tag_byte_array>().get();
    } catch (...) {
        std::cerr << "Error: Schematic file " << File << " loading error" << std::endl;
        return true;
    }

    return false;
}
