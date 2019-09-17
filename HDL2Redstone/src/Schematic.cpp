#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <io/izlibstream.h>
#include <io/stream_reader.h>
#include <nbt_tags.h>

#include <Exception.hpp>
#include <Schematic.hpp>

using namespace nbt;
using namespace HDL2Redstone;

// This comes from https://github.com/SpongePowered/Schematic-Specification
// We are only supporting version 2
static constexpr int SUPPORTED_SCHEM_VERSION = 2;
// We only support game version 1.14.4 for now
static constexpr int SUPPORTED_SCHEM_DATA_VERSION = 1976;
static constexpr auto SCHEM_VERSION = "Version";
static constexpr auto SCHEM_DATA_VERSION = "DataVersion";
static constexpr auto SCHEM_METADATA = "Metadata";
static constexpr auto SCHEM_WIDTH = "Width";
static constexpr auto SCHEM_HEIGHT = "Height";
static constexpr auto SCHEM_LENGTH = "Length";
static constexpr auto SCHEM_OFFSET = "Offset";
static constexpr auto SCHEM_PALETTE_MAX = "PaletteMax";
static constexpr auto SCHEM_PALETTE = "Palette";
static constexpr auto SCHEM_BLOCK_DATA = "BlockData";
static constexpr auto SCHEM_BLOCK_ENTITIES = "BlockEntities";
static constexpr auto SCHEM_ENTITIES = "Entities";
static constexpr auto SCHEM_BIOME_PALETTE_MAX = "BiomePaletteMax";
static constexpr auto SCHEM_BIOME_PALETTE = "BiomePalette";
static constexpr auto SCHEM_BIOME_DATA = "BiomeData";

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
        const auto& C = *P.second;

        if (C.at(SCHEM_VERSION).as<tag_int>() != SUPPORTED_SCHEM_VERSION) {
            throw Exception(File_ + " is not a schematic file with supported version " +
                            std::to_string(SUPPORTED_SCHEM_VERSION) + ".");
        }
        if (C.at(SCHEM_DATA_VERSION).as<tag_int>() != SUPPORTED_SCHEM_DATA_VERSION) {
            throw Exception(File_ + " is not a schematic file with supported data version " +
                            std::to_string(SUPPORTED_SCHEM_DATA_VERSION) + ".");
        }
        // We don't really care about Metadata
        Width = static_cast<uint16_t>(C.at(SCHEM_WIDTH).as<tag_short>());
        Height = static_cast<uint16_t>(C.at(SCHEM_HEIGHT).as<tag_short>());
        Length = static_cast<uint16_t>(C.at(SCHEM_LENGTH).as<tag_short>());
        // TODO: Based on the spec Offset, PaletteMax, Palette, and BlockEntities are not required fields. We shouldn't
        // need to error out.
        Offset = C.at(SCHEM_OFFSET).as<tag_int_array>().get();
        PaletteMax = C.at(SCHEM_PALETTE_MAX).as<tag_int>();
        const auto& PaletteMap = C.at(SCHEM_PALETTE).as<tag_compound>();
        for (auto It = PaletteMap.begin(); It != PaletteMap.end(); ++It) {
            InvertPalette.emplace(It->second, It->first);
        }
        int32_t Value;
        const auto& Blocks = C.at(SCHEM_BLOCK_DATA).as<tag_byte_array>();
        for (auto It = Blocks.begin(); It != Blocks.end();) {
            int32_t VarintLength = 0;
            Value = 0;
            while (true) {
                Value |= (static_cast<uint8_t>(*It) & 0x7F) << (VarintLength++ * 7);
                if (VarintLength > 5) {
                    throw Exception("VarintLength too big.");
                }
                if ((static_cast<uint8_t>(*It) & 0x80) != 0x80) {
                    ++It;
                    break;
                }
                ++It;
            }
            BlockData.push_back(Value);
        }
        // TODO: Add missing block entities data
        // C.at(SCHEM_BLOCK_ENTITIES).as<tag_compound>();
        // Ignore entities and biome related data
    } catch (Exception& e) {
        throw e;
    } catch (...) {
        throw Exception("Failed loading schematic file " + File_);
    }
}
