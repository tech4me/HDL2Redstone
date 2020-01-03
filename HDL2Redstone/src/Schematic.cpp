#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <io/izlibstream.h>
#include <io/ozlibstream.h>
#include <io/stream_reader.h>
#include <io/stream_writer.h>
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

Schematic::Schematic(uint16_t Width_, uint16_t Height_, uint16_t Length_)
    : Width(Width_), Height(Height_), Length(Length_), Offset{0, 0, 0},
      PaletteMax(1), InvertPalette{{0, "minecraft:air"}}, BlockData(Width_ * Height_ * Length_, 0),
      BlockOrigin(Width_ * Height_ * Length_, {"air", -1}) {}

Schematic::Schematic(const std::string& File_) {
    std::ifstream FS(File_, std::ios::binary);
    if (FS.fail()) {
        throw Exception("Problem opening File: " + File_);
    }

    try {
        zlib::izlibstream ZS(FS);
        auto P = io::read_compound(ZS);
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
        if (C.has_key(SCHEM_OFFSET, tag_type::Int_Array)) {
            Offset = C.at(SCHEM_OFFSET).as<tag_int_array>().get();
        } else {
            Offset = {0, 0, 0};
        }
        if (C.has_key(SCHEM_PALETTE_MAX, tag_type::Int)) {
            PaletteMax = C.at(SCHEM_PALETTE_MAX).as<tag_int>();
        } else {
            PaletteMax = 0;
        }
        if (C.has_key(SCHEM_PALETTE, tag_type::Compound)) {
            const auto& PaletteMap = C.at(SCHEM_PALETTE).as<tag_compound>();
            for (auto It = PaletteMap.begin(); It != PaletteMap.end(); ++It) {
                InvertPalette.emplace(It->second, It->first);
            }
        }
        int32_t Value;
        const auto& Blocks = C.at(SCHEM_BLOCK_DATA).as<tag_byte_array>();
        for (auto It = Blocks.begin(); It != Blocks.end();) {
            uint8_t Read;
            int32_t VarintLength = 0;
            Value = 0;
            do {
                Read = static_cast<uint8_t>(*It);
                Value |= (Read & 0x7F) << (VarintLength++ * 7);
                if (VarintLength > 5) {
                    throw Exception("VarintLength too big.");
                }
                ++It;
            } while ((Read & 0x80) != 0);
            BlockData.push_back(Value);
        }
        if (C.has_key(SCHEM_BLOCK_ENTITIES, tag_type::List)) {
            // TODO: check if the following works when actually have block entity data
            const auto& BlockEntities = C.at(SCHEM_BLOCK_ENTITIES).as<tag_list>();
            for (int i = 0; i < BlockEntities.size(); i++) {
                auto Entity = BlockEntities[i].as<tag_compound>();
                BlockEntityPositions.push_back(Entity.at("Pos").as<tag_int_array>().get());
                BlockEntityIds.push_back(Entity.at("Id").as<tag_string>().get());
                std::map<std::string, std::string> EntityText;
                for (auto It = Entity.begin(); It != Entity.end(); It++) {
                    if ((It->first != "Pos") && (It->first != "Id"))
                        EntityText.emplace(It->first, It->second);
                }
                BlockEntityTexts.push_back(EntityText);
            }
        }
        // Ignore entities and biome related data
    } catch (Exception& E) {
        throw E;
    } catch (...) {
        throw Exception("Failed reading schematic file " + File_);
    }
}

// Type_ is schematic cell type
// RouterSet_ is whether the block is set by Router or Placer, for debug
void Schematic::insertSubSchematic(const Placement& P_, const Schematic& Schem_, const std::string& Type_,
                                   const int32_t& RouterSet_) {
    // TODO: Add schematic out-of-bound checks here

    // 1. Merge sub schematic palette into schematic palette, update palette, create conversion map
    std::map<int32_t, int32_t> ConversionMap;
    for (int32_t i = 0; i < Schem_.InvertPalette.size(); ++i) {
        int32_t j = 0;
        for (; j < InvertPalette.size(); ++j) {
            if (Schem_.InvertPalette.at(i) == InvertPalette.at(j)) {
                // Add entry to conversion map
                ConversionMap.emplace(i, j);
                break;
            }
        }
        if (j == InvertPalette.size()) {
            // Insert a new entry into palette
            int32_t Loc = InvertPalette.size();
            InvertPalette.emplace(Loc, Schem_.InvertPalette.at(i));
            // Add entry to conversion map
            ConversionMap.emplace(i, Loc);
        }
    }
    // 2. Update palette max
    PaletteMax = InvertPalette.size();
    // 3. Update block data using conversion map
    for (int32_t i = 0; i < Schem_.BlockData.size(); ++i) {
        // TODO: Handle orientation here
        /*
        int32_t TempX = P_.X;
        int32_t TempY = P_.Y;
        int32_t TempZ = P_.Z;
        switch (P_.Orient) {
            case Orientation::OneCW:
            TempX = P_.Z;
            TempZ = -P_.X;
            break;
            case Orientation::TwoCW:
            TempX = -P_.X;
            TempZ = -P_.Z;
            break;
            case Orientation::ThreeCW:
            TempX = -P_.Z;
            TempZ = P_.X;
            break;
            default:
            break;
        }
        std::cout << TempX << std::endl;
        std::cout << TempY << std::endl;
        std::cout << TempZ << std::endl;
        */
        int32_t X = (i % (Schem_.Width * Schem_.Length)) % Schem_.Width;
        int32_t Y = i / (Schem_.Width * Schem_.Length);
        int32_t Z = (i % (Schem_.Width * Schem_.Length)) / Schem_.Width;
        int32_t Temp;
        switch (P_.Orient) {
        case Orientation::OneCW:
            Temp = X;
            X = Z;
            Z = -Temp;
            break;
        case Orientation::TwoCW:
            X = -X;
            Z = -Z;
            break;
        case Orientation::ThreeCW:
            Temp = X;
            X = -Z;
            Z = Temp;
            break;
        default:
            break;
        }
        X += P_.X;
        Y += P_.Y;
        Z += P_.Z;
        // Bound check
        if (X < 0 || X >= Width || Y < 0 || Y >= Height || Z < 0 || Z >= Length) {
            throw Exception("Block:" + Schem_.InvertPalette.at(i) + " X:" + std::to_string(X) +
                            " Y:" + std::to_string(Y) + " Z:" + std::to_string(Z) + " is being placed out of bound.");
        }
        int32_t Index = X + (Y * Width * Length) + (Z * Width);
        // Check overlap (non-air block)
        if (BlockData.at(Index)) {
            std::string Curr_src = RouterSet_ ? "Router" : "Placer";
            std::string Set_by = std::get<1>(BlockOrigin.at(Index)) ? "Router" : "Placer";
            std::string Cell_type = std::get<0>(BlockOrigin.at(Index));
            throw Exception("Block:" + Schem_.InvertPalette.at(Schem_.BlockData.at(i)) + " of cell " + Type_ +
                            " set by " + Curr_src + " cannot be placed at X:" + std::to_string(X) +
                            " Y:" + std::to_string(Y) + " Z:" + std::to_string(Z) +
                            " ; location already occupied by cell " + Cell_type + " set by " + Set_by + ".");
        }
        auto It = ConversionMap.find(Schem_.BlockData.at(i));
        if (It != ConversionMap.end()) {
            BlockData.at(Index) = It->second;
        } else {
            BlockData.at(Index) = Schem_.BlockData.at(i);
        }
        // for debug
        BlockOrigin.at(Index) = std::make_tuple(Type_, RouterSet_);
    }
    // TODO: not checked against real block entity info!
    for (int32_t i = 0; i < Schem_.BlockEntityPositions.size(); ++i) {
        std::vector<int32_t> Pos = BlockEntityPositions[i];
        std::vector<int32_t> Updated_pos({Pos[0] + P_.X, Pos[1] + P_.Y, Pos[2] + P_.Z});
        BlockEntityPositions.push_back(Updated_pos);
        BlockEntityIds.push_back(Schem_.BlockEntityIds[i]);
        BlockEntityTexts.push_back(Schem_.BlockEntityTexts[i]);
    }
}

void Schematic::exportSchematic(const std::string& File_) const {
    std::ofstream FS(File_, std::ios::binary);
    if (FS.fail()) {
        throw Exception("Problem opening File: " + File_);
    }

    try {
        // Output in gzip format for WorldEdit
        zlib::ozlibstream ZS(FS, Z_DEFAULT_COMPRESSION, true);
        tag_compound C;
        C.emplace<tag_int>(SCHEM_VERSION, SUPPORTED_SCHEM_VERSION);
        C.emplace<tag_int>(SCHEM_DATA_VERSION, SUPPORTED_SCHEM_DATA_VERSION);
        C.emplace<tag_short>(SCHEM_WIDTH, Width);
        C.emplace<tag_short>(SCHEM_HEIGHT, Height);
        C.emplace<tag_short>(SCHEM_LENGTH, Length);

        C.emplace<tag_int_array>(SCHEM_OFFSET, tag_int_array(std::vector<int32_t>(Offset)));
        C.emplace<tag_int>(SCHEM_PALETTE_MAX, PaletteMax);
        tag_compound Palette;
        for (auto It = InvertPalette.begin(); It != InvertPalette.end(); ++It) {
            Palette.emplace<tag_int>(It->second, It->first);
        }
        C.emplace<tag_compound>(SCHEM_PALETTE, Palette);
        std::vector<int8_t> Blocks;
        int i = 0;
        for (auto It = BlockData.begin(); It != BlockData.end(); ++It) {
            int32_t Value;
            do {
                Value = *It;
                uint8_t Temp = static_cast<int8_t>(Value & 0x7F);
                // Cast to unsigned int to avoid sign extension
                Value = static_cast<uint32_t>(Value) >> 7;
                if (Value != 0) {
                    Temp |= 0x80;
                }
                Blocks.push_back(Temp);
            } while (Value != 0);
            i++;
        }
        C.emplace<tag_byte_array>(SCHEM_BLOCK_DATA, tag_byte_array(std::move(Blocks)));
        // TODO: Not Tested....
        tag_list BlockEntities;
        for (int32_t i = 0; i < BlockEntityPositions.size(); i++) {
            tag_compound BlockEntity;
            BlockEntity.emplace<tag_int_array>("Pos", tag_int_array(std::vector<int32_t>(BlockEntityPositions[i])));
            BlockEntity.emplace<tag_string>("Id", tag_string(BlockEntityIds[i]));
            for (auto It = BlockEntityTexts[i].begin(); It != BlockEntityTexts[i].end(); It++)
                BlockEntity.emplace<tag_string>(tag_string(It->first), tag_string(It->second));
            BlockEntities.push_back(tag_compound(BlockEntity));
        }
        C.emplace<tag_list>(SCHEM_BLOCK_ENTITIES, BlockEntities);
        // END Not Tested....
        io::write_tag("Schematic", C, ZS);
    } catch (...) {
        throw Exception("Failed writing schematic file " + File_);
    }
}

namespace HDL2Redstone {
std::ostream& operator<<(std::ostream& out, const Schematic& Schematic_) {
    uint16_t w = Schematic_.Width;
    uint16_t l = Schematic_.Length;
    // Print L, W, H
    // Print Offset {X, Y ,Z}
    // for index in range(0, BlockData.size())
    // 	     -> Location {X, Y, Z} // Calculate from index
    // 	     -> Type IntertPalette.at(BlockData.at(index))
    out << "  Length: " << Schematic_.Length << ", Weight: " << Schematic_.Width << ", Height: " << Schematic_.Height
        << std::endl;

    out << "  Offset: ";
    for (const auto o : Schematic_.Offset)
        out << o << ", ";
    out << std::endl;

    for (int i = 0; i < Schematic_.BlockData.size(); i++) {
        out << "  Location: X:" << (i % (w * l)) % w << " Y:" << i / (w * l) << " Z:" << i % (w * l) / w << ";  ";
        out << "  Type: " << Schematic_.InvertPalette.at(Schematic_.BlockData.at(i)) << std::endl;
    }
    return out;
}
} // namespace HDL2Redstone
