#include <fstream>
#include <iostream>
#include <regex>
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
// We only support version 2
static constexpr int SUPPORTED_SCHEM_VERSION = 2;
// We only support game version 1.15.2
static constexpr int SUPPORTED_SCHEM_DATA_VERSION = 2230;
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
            const auto& BlockEntities = C.at(SCHEM_BLOCK_ENTITIES).as<tag_list>();
            for (const auto& NBTEntity : BlockEntities) {
                const auto& Entity = NBTEntity.as<tag_compound>();
                BlockEntityPositions.push_back(Entity.at("Pos").as<tag_int_array>().get());
                BlockEntityIds.push_back(Entity.at("Id").as<tag_string>().get());
                std::map<std::string, std::string> EntityExtra;
                for (auto It = Entity.begin(); It != Entity.end(); ++It) {
                    // std::cout<<It->first<<" "<<It->second<<std::endl;
                    if ((It->first == "Pos") || (It->first == "Id")) {
                        continue;
                    }
                    if (It->first == "OutputSignal") { // for comparator
                        EntityExtra.emplace(It->first, std::to_string(It->second.as<tag_int>()));
                    } else if (It->first == "Color" || It->first == "Text1" || It->first == "Text2" ||
                               It->first == "Text3" || It->first == "Text4") { // for sign
                        EntityExtra.emplace(It->first, It->second);
                    } else {
                        throw Exception("Unkown block entity field: " + It->first);
                    }
                }
                BlockEntityExtras.push_back(EntityExtra);
            }
        }
        // Ignore biome related data
    } catch (Exception& E) {
        throw E;
    } catch (...) {
        throw Exception("Failed reading schematic file " + File_);
    }
}

// Type_ is schematic cell type
// RouterSet_ is whether the block is set by Router or Placer, for debug
void Schematic::insertSubSchematic(const Placement& P_, const Schematic& Schem_, const std::string& Type_,
                                   const int32_t& RouterSet_, const std::string& Name_) {
    // TODO: Add schematic out-of-bound checks here

    // 1. Merge sub schematic palette into schematic palette, update palette, create conversion map
    std::map<int32_t, int32_t> ConversionMap;
    for (int32_t i = 0; i < Schem_.InvertPalette.size(); ++i) {
        // perform necessary changes caused by rotation to palette
        std::string Str = Schem_.InvertPalette.at(i);

        std::regex NorthReg("(north=)(\\w+),");
        std::regex SouthReg("(south=)(\\w+),");
        std::regex EastReg("(east=)(\\w+),");
        std::regex WestReg("(west=)(\\w+)]");
        std::regex FaceReg("(facing=)(\\w+),");
        std::smatch Nmatch, Smatch, Ematch, Wmatch, Fmatch;

        // redstone wire
        if (std::regex_search(Str, Nmatch, NorthReg) && std::regex_search(Str, Smatch, SouthReg) &&
            std::regex_search(Str, Ematch, EastReg) && std::regex_search(Str, Wmatch, WestReg)) {
            switch (P_.Orient) {
            case Orientation::OneCW:
                Str = std::regex_replace(Str, EastReg, "$1" + Nmatch.str(2) + ",");
                Str = std::regex_replace(Str, SouthReg, "$1" + Ematch.str(2) + ",");
                Str = std::regex_replace(Str, WestReg, "$1" + Smatch.str(2) + "]");
                Str = std::regex_replace(Str, NorthReg, "$1" + Wmatch.str(2) + ",");
                break;
            case Orientation::TwoCW:
                Str = std::regex_replace(Str, EastReg, "$1" + Wmatch.str(2) + ",");
                Str = std::regex_replace(Str, SouthReg, "$1" + Nmatch.str(2) + ",");
                Str = std::regex_replace(Str, WestReg, "$1" + Ematch.str(2) + "]");
                Str = std::regex_replace(Str, NorthReg, "$1" + Smatch.str(2) + ",");
                break;
            case Orientation::ThreeCW:
                Str = std::regex_replace(Str, EastReg, "$1" + Smatch.str(2) + ",");
                Str = std::regex_replace(Str, SouthReg, "$1" + Wmatch.str(2) + ",");
                Str = std::regex_replace(Str, WestReg, "$1" + Nmatch.str(2) + "]");
                Str = std::regex_replace(Str, NorthReg, "$1" + Ematch.str(2) + ",");
                break;
            default:
                // std::cout<<"NO matching placement orientation: "<<(int)P_.Orient<<std::endl;
                break;
            }
            // redstone_torch or repeater, or sign
        } else if (std::regex_search(Str, Fmatch, FaceReg)) {
            std::string Facing = Fmatch.str(2);
            uint32_t FInt = 0, turn = 0;

            if (Facing == "north")
                FInt = 1;
            else if (Facing == "east")
                FInt = 2;
            else if (Facing == "south")
                FInt = 3;

            switch (P_.Orient) {
            case Orientation::OneCW:
                turn = 1;
                break;
            case Orientation::TwoCW:
                turn = 2;
                break;
            case Orientation::ThreeCW:
                turn = 3;
                break;
            default:
                break;
            }

            FInt = (FInt + turn) % 4;
            switch (FInt) {
            case 0:
                Facing = "facing=west,";
                break;
            case 1:
                Facing = "facing=north,";
                break;
            case 2:
                Facing = "facing=east,";
                break;
            case 3:
                Facing = "facing=south,";
                break;
            default:
                break;
            }
            Str = std::regex_replace(Str, FaceReg, Facing);
        }
        if (Str != Schem_.InvertPalette.at(i))
            DOUT(<< "  changed from " << Schem_.InvertPalette.at(i) << " to " << Str << std::endl);

        int32_t j = 0;
        for (; j < InvertPalette.size(); ++j) {
            if (/*Schem_.InvertPalette.at(i)*/ Str == InvertPalette.at(j)) {
                // Add entry to conversion map
                ConversionMap.emplace(i, j);
                break;
            }
        }
        if (j == InvertPalette.size()) {
            // Insert a new entry into palette
            int32_t Loc = InvertPalette.size();
            InvertPalette.emplace(Loc, /*Schem_.InvertPalette.at(i)*/ Str);
            // Add entry to conversion map
            ConversionMap.emplace(i, Loc);
        }
    }
    // 2. Update palette max
    PaletteMax = InvertPalette.size();
    // 3. Update block data using conversion map
    for (int32_t i = 0; i < Schem_.BlockData.size(); ++i) {
        int32_t X = (i % (Schem_.Width * Schem_.Length)) % Schem_.Width;
        int32_t Y = i / (Schem_.Width * Schem_.Length);
        int32_t Z = (i % (Schem_.Width * Schem_.Length)) / Schem_.Width;
        int32_t Temp;
        // std::cout<<"old:"<<X<<" "<<Y<<" "<<Z<<std::endl;
        switch (P_.Orient) {
        case Orientation::OneCW:
            Temp = X;
            X = -Z - 1;
            Z = Temp;
            break;
        case Orientation::TwoCW:
            X = -X - 1;
            Z = -Z - 1;
            break;
        case Orientation::ThreeCW:
            Temp = X;
            X = Z;
            Z = -Temp - 1;
            break;
        default:
            break;
        }
        // std::cout<<"after rot:"<<X<<" "<<Y<<" "<<Z<<std::endl;
        X += P_.X;
        Y += P_.Y;
        Z += P_.Z;
        // std::cout<<"p added:"<<X<<" "<<Y<<" "<<Z<<std::endl;
        // Bound check
        if (X < 0 || X >= Width || Y < 0 || Y >= Height || Z < 0 || Z >= Length) {
            // TODO: invertPalette.at(i) prob wrong
            throw Exception("Block:" + Schem_.InvertPalette.at(i) + " X:" + std::to_string(X) +
                            " Y:" + std::to_string(Y) + " Z:" + std::to_string(Z) + " is being placed out of bound.");
        }
        int32_t Index = X + (Y * Width * Length) + (Z * Width);
        // Check overlap (non-air block)
        if (BlockData.at(Index)) {
            std::string Curr_src = RouterSet_ ? "Router" : "Placer";
            std::string Set_by = std::get<1>(BlockOrigin.at(Index)) ? "Router" : "Placer";
            std::string Cell_type = std::get<0>(BlockOrigin.at(Index));
            // TODO: block palette could be different after rotation, will need to make Str available here
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
    for (int32_t i = 0; i < Schem_.BlockEntityPositions.size(); ++i) {
        std::vector<int32_t> Pos = Schem_.BlockEntityPositions[i];
        int32_t Temp;
        // rotate entities, code below could be put into a func...
        switch (P_.Orient) {
        case Orientation::OneCW:
            Temp = Pos[0];
            Pos[0] = Pos[2] - 1;
            Pos[2] = Temp;
            break;
        case Orientation::TwoCW:
            Pos[0] = -Pos[0] - 1;
            Pos[2] = -Pos[2] - 1;
            break;
        case Orientation::ThreeCW:
            Temp = Pos[0];
            Pos[0] = Pos[2];
            Pos[2] = -Temp - 1;
            break;
        default:
            break;
        }
        std::vector<int32_t> Updated_pos({Pos[0] + P_.X, Pos[1] + P_.Y, Pos[2] + P_.Z});
        BlockEntityPositions.push_back(Updated_pos);
        BlockEntityIds.push_back(Schem_.BlockEntityIds[i]);
        std::map<std::string, std::string> EntityExtras;
        if (Schem_.BlockEntityIds[i] == "minecraft:sign") {
            // std::cout<<"inserting schem, find sign"<<std::endl; ///////////////////////////////////
            std::string text = "{\"text\":\"";
            for (const auto& [key, value] : Schem_.BlockEntityExtras[i]) {
                if (key == "Text1") {
                    EntityExtras.emplace("Text1", text + Type_ + "\"}");
                } else if (key == "Text3") {
                    EntityExtras.emplace("Text3", text + "\\u00A7c" + Name_ + "\"}");
                } else {
                    EntityExtras.emplace(key, value);
                }
            }
        }
        //	for (auto en : EntityExtras) {std::cout<<en.first<<" "<<en.second<<std::endl;}
        BlockEntityExtras.push_back(EntityExtras);
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
        tag_list BlockEntities;
        for (int32_t i = 0; i < BlockEntityPositions.size(); i++) {
            tag_compound BlockEntity;
            BlockEntity.emplace<tag_int_array>("Pos", tag_int_array(std::vector<int32_t>(BlockEntityPositions[i])));
            BlockEntity.emplace<tag_string>("Id", tag_string(BlockEntityIds[i]));
            for (auto It = BlockEntityExtras[i].begin(); It != BlockEntityExtras[i].end(); ++It) {
                if ((It->first == "Pos") || (It->first == "Id")) {
                    continue;
                }
                if (It->first == "OutputSignal") { // for comparator
                    BlockEntity.emplace<tag_int>(tag_string(It->first), tag_int(std::stoi(It->second)));
                } else if (It->first == "Color" || It->first == "Text1" || It->first == "Text2" ||
                           It->first == "Text3" || It->first == "Text4") { // for sign
                    // std::cout<<"exporting block entity!"<<std::endl;
                    BlockEntity.emplace<tag_string>(tag_string(It->first), tag_string(It->second));
                }
            }
            BlockEntities.push_back(tag_compound(BlockEntity));
        }
        C.emplace<tag_list>(SCHEM_BLOCK_ENTITIES, BlockEntities);
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
