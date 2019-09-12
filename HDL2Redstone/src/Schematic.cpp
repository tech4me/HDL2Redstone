#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <io/izlibstream.h>
#include <io/stream_reader.h>
#include <nbt_tags.h>

#include <Exception.hpp>
#include <Schematic.hpp>

using namespace HDL2Redstone;

// This comes from https://minecraft.gamepedia.com/Schematic_file_format
static constexpr auto SCH_WIDTH = "Width";
static constexpr auto SCH_HEIGHT = "Height";
static constexpr auto SCH_LENGTH = "Length";
static constexpr auto SCH_MATERIALS = "Materials";
static constexpr auto SCH_BLOCKS = "Blocks";
static constexpr auto SCH_DATA = "Data";
static constexpr auto SCH_ENTITIES = "Entities";
static constexpr auto SCH_TILE_ENTITIES = "TileEntities";

Schematic::Schematic(const std::string& File_) {
    std::ifstream FS(File_, std::ios::binary);
    if (FS.fail()) {
        throw Exception("Problem opening File: " + File_);
    }

    try {
        zlib::izlibstream ZS(FS);
        auto P = nbt::io::read_compound(ZS);
        if (P.first != "Schematic") {
            throw Exception(File_ + " is not a schematic file.");
        }
        auto C = *P.second;

        Width = C.at(SCH_WIDTH).as<nbt::tag_short>();
        Height = C.at(SCH_HEIGHT).as<nbt::tag_short>();
        Length = C.at(SCH_LENGTH).as<nbt::tag_short>();
        Materials = C.at(SCH_MATERIALS).as<nbt::tag_string>();
        Blocks = C.at(SCH_BLOCKS).as<nbt::tag_byte_array>().get();
        Data = C.at(SCH_DATA).as<nbt::tag_byte_array>().get();
    } catch (...) {
        throw Exception("Failed loading schematic file " + File_);
    }
}
