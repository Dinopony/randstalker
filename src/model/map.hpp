#pragma once

#include "../tools/megadrive/rom.hpp"
#include "../extlibs/json.hpp"

class EntityOnMap;
class World;

struct MapExit {
    uint8_t pos_x;
    uint8_t pos_y;

    uint16_t destination_map_id;
    uint8_t destination_x;
    uint8_t destination_y;

    /// 0x1 => Initially closed?
    /// 0x2 => NE
    /// 0x4 => NW
    /// 0x8 => ???
    /// 0x10 => NE Stairs
    /// 0x20 => NW Stairs
    uint8_t extra_byte;

    Json to_json() const {
        Json json;
        json["posX"] = pos_x;
        json["posY"] = pos_y;
        json["extraByte"] = extra_byte;
        json["destinationMapId"] = destination_map_id;
        json["destinationX"] = destination_x;
        json["destinationY"] = destination_y;
        return json;
    }
};

struct MapVariant {
    uint16_t map_variant_id;
    uint8_t flag_byte;
    uint8_t flag_bit;

    Json to_json() const {
        Json json;
        json["mapVariantId"] = map_variant_id;
        json["flagByte"] = flag_byte;
        json["flagBit"] = flag_bit;
        return json;
    }
};

struct Flag {
    uint8_t byte;
    uint8_t bit;

    Flag() : byte(0), bit(0)
    {}

    Flag(uint8_t byte, uint8_t bit) : byte(byte), bit(bit)
    {}
};

struct EntityMask {
    bool visibility_if_flag_set;
    Flag flag;
    uint8_t entity_id;

    EntityMask(bool p_visibility_if_flag_set, uint8_t flag_byte, uint8_t flag_bit, uint8_t p_entity_id) :
        visibility_if_flag_set  (p_visibility_if_flag_set),
        flag                    (flag_byte, flag_bit),
        entity_id               (p_entity_id)
    {}

    EntityMask(uint16_t word)
    {
        uint8_t msb = word >> 8;
        uint8_t lsb = word & 0x00FF;

        visibility_if_flag_set = msb >> 7;
        flag.byte = msb & 0x7F;
        flag.bit = lsb >> 5;
        entity_id = lsb & 0x0F;
    }

    uint16_t to_bytes() const
    {
        uint8_t msb = flag.byte & 0x7F;
        if(visibility_if_flag_set)
            msb |= 0x80;

        uint8_t lsb = entity_id & 0x0F;
        lsb |= (flag.bit & 0x7) << 5;
        
        return (((uint16_t)msb) << 8) + lsb;
    }

    Json to_json() const
    {
        Json json;
        json["ifFlagSet"] = visibility_if_flag_set ? "show" : "hide";
        json["flagByte"] = flag.byte;
        json["flagBit"] = flag.bit;
        json["entityId"] = entity_id;
        return json;
    }
};

struct ClearFlag {
    Flag flag;
    uint8_t first_entity_id;
    
    ClearFlag(uint16_t word)
    {
        uint8_t msb = word >> 8;
        uint8_t lsb = word & 0x00FF;

        flag.byte = msb;
        flag.bit = lsb >> 5;
        first_entity_id = lsb & 0x1F;
    }

    uint16_t to_bytes() const
    {
        uint8_t msb = flag.byte;
        uint8_t lsb = first_entity_id & 0x1F;
        lsb |= (flag.bit & 0x7) << 5;

        return (((uint16_t)msb) << 8) + lsb;
    }

    Json to_json() const
    {
        Json json;
        json["flagByte"] = flag.byte;
        json["flagBit"] = flag.bit;
        json["firstEntityId"] = first_entity_id;
        return json;
    }
};

class Map
{
private:
    uint32_t _id;
    uint32_t _address;

    uint8_t _tileset_id;
    uint8_t _primary_big_tileset_id;
    uint8_t _secondary_big_tileset_id;
    uint8_t _big_tileset_id;

    uint8_t _palette_id;
    uint8_t _room_height;
    uint8_t _background_music;

    uint8_t _unknown_param_1;
    uint8_t _unknown_param_2;

    uint8_t _base_chest_id;
    uint16_t _fall_destination;
    uint16_t _climb_destination;
    
    std::vector<EntityOnMap*> _entities;
    std::vector<MapExit> _exits;
    std::vector<MapVariant> _variants;
    std::vector<EntityMask> _entity_masks;
    std::vector<ClearFlag> _clear_flags;

public:
    Map(uint16_t map_id, const md::ROM& rom, const World& world);
    Map(const Map& map);
    ~Map();
    
    void clear();
    void write_to_rom(md::ROM& rom);

    void id(uint16_t id) { _id = id; }
    uint16_t id() const { return _id; }

    uint8_t base_chest_id() const { return _base_chest_id; }
    void base_chest_id(uint8_t id) { _base_chest_id = id; }

    uint16_t fall_destination() const { return _fall_destination; }
    uint16_t climb_destination() const { return _climb_destination; }

    uint8_t background_music() const { return _background_music; }
    void background_music(uint8_t music) { _background_music = music; }

    const std::vector<EntityOnMap*>& entities() const { return _entities; }
    const EntityOnMap& entity(uint8_t entity_id) const { return *_entities.at(entity_id); }
    EntityOnMap& entity(uint8_t entity_id) { return *_entities.at(entity_id); }
    uint8_t add_entity(EntityOnMap* entity);
    void remove_entity(uint8_t entity_id);
    void clear_entities();

    const std::vector<MapExit>& exits() const { return _exits; }
    std::vector<MapExit>& exits() { return _exits; }

    const std::vector<MapVariant>& variants() const { return _variants; }
    std::vector<MapVariant>& variants() { return _variants; }

    const std::vector<EntityMask>& entity_masks() const { return _entity_masks; }
    std::vector<EntityMask>& entity_masks() { return _entity_masks; }

    const std::vector<ClearFlag>& clear_flags() const { return _clear_flags; }
    std::vector<ClearFlag>& clear_flags() { return _clear_flags; }

    void add_variant(uint16_t map_variant_id, uint8_t flag_byte, uint8_t flag_bit)
    {
        MapVariant new_variant;
        new_variant.map_variant_id = map_variant_id;
        new_variant.flag_byte = flag_byte;
        new_variant.flag_bit = flag_bit;
        _variants.push_back(new_variant);
    }

    MapVariant& variant(uint16_t map_variant_id)
    {
        for(MapVariant& variant : _variants)
            if(variant.map_variant_id == map_variant_id)
                return variant;
        throw std::out_of_range("No variant with given ID");
    }

    std::vector<uint8_t> entities_as_bytes() const;

    Json to_json(const World& world) const;
    Map* from_json(const Json& json);

private:
    void read_map_data(const md::ROM& rom);
    void write_map_data(md::ROM& rom);

    void read_base_chest_id(const md::ROM& rom);
    void write_base_chest_id(md::ROM& rom);

    void read_fall_destination(const md::ROM& rom);
    void write_fall_destination(md::ROM& rom);

    void read_climb_destination(const md::ROM& rom);
    void write_climb_destination(md::ROM& rom);

    void read_entities(const md::ROM& rom, const World& world);
    void read_exits(const md::ROM& rom);
    void read_variants(const md::ROM& rom);
    void read_entity_masks(const md::ROM& rom);
    void read_clear_flags(const md::ROM& rom);
};

constexpr uint16_t MAP_SWAMP_SHRINE_0 = 0;
constexpr uint16_t MAP_SWAMP_SHRINE_1 = 1;
constexpr uint16_t MAP_SWAMP_SHRINE_2 = 2;
constexpr uint16_t MAP_SWAMP_SHRINE_3 = 3;
constexpr uint16_t MAP_SWAMP_SHRINE_4 = 4;
constexpr uint16_t MAP_SWAMP_SHRINE_5 = 5;
constexpr uint16_t MAP_SWAMP_SHRINE_6 = 6;
constexpr uint16_t MAP_SWAMP_SHRINE_7 = 7;
constexpr uint16_t MAP_SWAMP_SHRINE_8 = 8;
constexpr uint16_t MAP_SWAMP_SHRINE_9 = 9;
constexpr uint16_t MAP_SWAMP_SHRINE_10 = 10;
constexpr uint16_t MAP_SWAMP_SHRINE_11 = 11;
constexpr uint16_t MAP_SWAMP_SHRINE_12 = 12;
constexpr uint16_t MAP_SWAMP_SHRINE_13 = 13;
constexpr uint16_t MAP_SWAMP_SHRINE_14 = 14;
constexpr uint16_t MAP_SWAMP_SHRINE_15 = 15;
constexpr uint16_t MAP_SWAMP_SHRINE_16 = 16;
constexpr uint16_t MAP_SWAMP_SHRINE_17 = 17;
constexpr uint16_t MAP_SWAMP_SHRINE_18 = 18;
constexpr uint16_t MAP_SWAMP_SHRINE_19 = 19;
constexpr uint16_t MAP_SWAMP_SHRINE_20 = 20;
constexpr uint16_t MAP_SWAMP_SHRINE_21 = 21;
constexpr uint16_t MAP_SWAMP_SHRINE_22 = 22;
constexpr uint16_t MAP_SWAMP_SHRINE_23 = 23;
constexpr uint16_t MAP_SWAMP_SHRINE_24 = 24;
constexpr uint16_t MAP_SWAMP_SHRINE_25 = 25;
constexpr uint16_t MAP_SWAMP_SHRINE_26 = 26;
constexpr uint16_t MAP_SWAMP_SHRINE_27 = 27;
constexpr uint16_t MAP_SWAMP_SHRINE_BOSS_ROOM = 28;
constexpr uint16_t MAP_SWAMP_SHRINE_29 = 29;
constexpr uint16_t MAP_SWAMP_SHRINE_30 = 30;

// Mercator castle & dungeon
constexpr uint16_t MAP_31 = 31;
constexpr uint16_t MAP_MERCATOR_CASTLE_LEFT_COURT = 32;
constexpr uint16_t MAP_MERCATOR_CASTLE_LEFT_COURT_RIBBONLESS_VARIANT = 33;
constexpr uint16_t MAP_34 = 34;
constexpr uint16_t MAP_35 = 35;
constexpr uint16_t MAP_36 = 36;
constexpr uint16_t MAP_37 = 37;
constexpr uint16_t MAP_38 = 38;
constexpr uint16_t MAP_39 = 39;
constexpr uint16_t MAP_40 = 40;
constexpr uint16_t MAP_41 = 41;
constexpr uint16_t MAP_42 = 42;
constexpr uint16_t MAP_43 = 43;
constexpr uint16_t MAP_44 = 44;
constexpr uint16_t MAP_45 = 45;
constexpr uint16_t MAP_46 = 46;
constexpr uint16_t MAP_47 = 47;
constexpr uint16_t MAP_48 = 48;
constexpr uint16_t MAP_49 = 49;
constexpr uint16_t MAP_50 = 50;
constexpr uint16_t MAP_51 = 51;
constexpr uint16_t MAP_52 = 52;
constexpr uint16_t MAP_53 = 53;
constexpr uint16_t MAP_MERCATOR_CASTLE_THRONE_ROOM_ARTHUR_VARIANT = 54;
constexpr uint16_t MAP_55 = 55;
constexpr uint16_t MAP_56 = 56;
constexpr uint16_t MAP_57 = 57;
constexpr uint16_t MAP_58 = 58;
constexpr uint16_t MAP_59 = 59;
constexpr uint16_t MAP_60 = 60;
constexpr uint16_t MAP_61 = 61;
constexpr uint16_t MAP_62 = 62;
constexpr uint16_t MAP_63 = 63;
constexpr uint16_t MAP_64 = 64;
constexpr uint16_t MAP_65 = 65;
constexpr uint16_t MAP_66 = 66;
constexpr uint16_t MAP_67 = 67;
constexpr uint16_t MAP_68 = 68;
constexpr uint16_t MAP_69 = 69;
constexpr uint16_t MAP_70 = 70;
constexpr uint16_t MAP_71 = 71;
constexpr uint16_t MAP_72 = 72;
constexpr uint16_t MAP_73 = 73;
constexpr uint16_t MAP_74 = 74;
constexpr uint16_t MAP_75 = 75;
constexpr uint16_t MAP_76 = 76;
constexpr uint16_t MAP_77 = 77;
constexpr uint16_t MAP_78 = 78;
constexpr uint16_t MAP_79 = 79;
constexpr uint16_t MAP_80 = 80;
constexpr uint16_t MAP_81 = 81;
constexpr uint16_t MAP_82 = 82;
constexpr uint16_t MAP_83 = 83;
constexpr uint16_t MAP_MERCATOR_CASTLE_ARMORY_1F = 84;
constexpr uint16_t MAP_85 = 85;
constexpr uint16_t MAP_86 = 86;
constexpr uint16_t MAP_87 = 87;
constexpr uint16_t MAP_88 = 88;
constexpr uint16_t MAP_89 = 89;
constexpr uint16_t MAP_90 = 90;
constexpr uint16_t MAP_91 = 91;
constexpr uint16_t MAP_92 = 92;
constexpr uint16_t MAP_93 = 93;
constexpr uint16_t MAP_94 = 94;
constexpr uint16_t MAP_95 = 95;
constexpr uint16_t MAP_96 = 96;
constexpr uint16_t MAP_97 = 97;
constexpr uint16_t MAP_98 = 98;
constexpr uint16_t MAP_99 = 99;
constexpr uint16_t MAP_100 = 100;
constexpr uint16_t MAP_101 = 101;
constexpr uint16_t MAP_102 = 102;
constexpr uint16_t MAP_103 = 103;
constexpr uint16_t MAP_104 = 104;
constexpr uint16_t MAP_105 = 105;
constexpr uint16_t MAP_106 = 106;
constexpr uint16_t MAP_107 = 107;
constexpr uint16_t MAP_108 = 108;
constexpr uint16_t MAP_109 = 109;
constexpr uint16_t MAP_110 = 110;

constexpr uint16_t MAP_NOLE_ARENA = 111;
constexpr uint16_t MAP_GOLA_ARENA = 112;
constexpr uint16_t MAP_GOLA_ARENA_CINEMATIC_VARIANT = 113;
constexpr uint16_t MAP_PILLAR_HALLWAY_TO_NOLE = 114;

constexpr uint16_t MAP_KN_PALACE_115 = 115;
constexpr uint16_t MAP_KN_PALACE_116 = 116;
constexpr uint16_t MAP_KN_PALACE_117 = 117;
constexpr uint16_t MAP_KN_PALACE_118 = 118;
constexpr uint16_t MAP_KN_PALACE_119 = 119;
constexpr uint16_t MAP_KN_PALACE_120 = 120;
constexpr uint16_t MAP_KN_PALACE_121 = 121;
constexpr uint16_t MAP_KN_PALACE_122 = 122;
constexpr uint16_t MAP_KN_PALACE_123 = 123;
constexpr uint16_t MAP_KN_PALACE_124 = 124;
constexpr uint16_t MAP_KN_PALACE_125 = 125;
constexpr uint16_t MAP_KN_PALACE_126 = 126;
constexpr uint16_t MAP_KN_PALACE_127 = 127;
constexpr uint16_t MAP_KN_PALACE_128 = 128;
constexpr uint16_t MAP_KN_PALACE_129 = 129;
constexpr uint16_t MAP_KN_PALACE_130 = 130;
constexpr uint16_t MAP_KN_PALACE_131 = 131;
constexpr uint16_t MAP_KN_PALACE_132 = 132;
constexpr uint16_t MAP_KN_PALACE_133 = 133;
constexpr uint16_t MAP_KN_PALACE_134 = 134;
constexpr uint16_t MAP_KN_PALACE_135 = 135;
constexpr uint16_t MAP_KN_PALACE_136 = 136;
constexpr uint16_t MAP_KN_PALACE_137 = 137;
constexpr uint16_t MAP_KN_PALACE_138 = 138;

constexpr uint16_t MAP_INTRO_139 = 139;
constexpr uint16_t MAP_INTRO_140 = 140;
constexpr uint16_t MAP_INTRO_141 = 141;
constexpr uint16_t MAP_INTRO_142 = 142;
constexpr uint16_t MAP_INTRO_143 = 143;

constexpr uint16_t MAP_KN_CAVE_144 = 144;
constexpr uint16_t MAP_KN_CAVE_145 = 145;
constexpr uint16_t MAP_KN_CAVE_146 = 146;
constexpr uint16_t MAP_KN_CAVE_147 = 147;
constexpr uint16_t MAP_KN_CAVE_148 = 148;
constexpr uint16_t MAP_KN_CAVE_149 = 149;
constexpr uint16_t MAP_KN_CAVE_150 = 150;
constexpr uint16_t MAP_KN_CAVE_151 = 151;
constexpr uint16_t MAP_KN_CAVE_152 = 152;
constexpr uint16_t MAP_KN_CAVE_153 = 153;
constexpr uint16_t MAP_KN_CAVE_154 = 154;
constexpr uint16_t MAP_KN_CAVE_155 = 155;
constexpr uint16_t MAP_KN_CAVE_156 = 156;
constexpr uint16_t MAP_KN_CAVE_157 = 157;
constexpr uint16_t MAP_KN_CAVE_158 = 158;
constexpr uint16_t MAP_KN_CAVE_159 = 159;
constexpr uint16_t MAP_KN_CAVE_160 = 160;
constexpr uint16_t MAP_KN_CAVE_161 = 161;
constexpr uint16_t MAP_KN_CAVE_162 = 162;
constexpr uint16_t MAP_KN_CAVE_163 = 163;
constexpr uint16_t MAP_KN_CAVE_164 = 164;
constexpr uint16_t MAP_KN_CAVE_165 = 165;
constexpr uint16_t MAP_KN_CAVE_166 = 166;
constexpr uint16_t MAP_KN_CAVE_167 = 167;
constexpr uint16_t MAP_KN_CAVE_168 = 168;
constexpr uint16_t MAP_KN_CAVE_169 = 169;
constexpr uint16_t MAP_KN_CAVE_TELEPORTER_TO_KAZALT = 170;
constexpr uint16_t MAP_KN_CAVE_171 = 171;
constexpr uint16_t MAP_KN_CAVE_172 = 172;
constexpr uint16_t MAP_KAZALT_TELEPORTER_TO_KN_CAVE = 173;

constexpr uint16_t MAP_WATERFALL_SHRINE_174 = 174;
constexpr uint16_t MAP_WATERFALL_SHRINE_175 = 175;
constexpr uint16_t MAP_WATERFALL_SHRINE_176 = 176;
constexpr uint16_t MAP_WATERFALL_SHRINE_177 = 177;
constexpr uint16_t MAP_WATERFALL_SHRINE_178 = 178;
constexpr uint16_t MAP_WATERFALL_SHRINE_179 = 179;
constexpr uint16_t MAP_WATERFALL_SHRINE_180 = 180;
constexpr uint16_t MAP_WATERFALL_SHRINE_181 = 181;
constexpr uint16_t MAP_WATERFALL_SHRINE_182 = 182;

constexpr uint16_t MAP_RYUMA_RAFT_COAST = 183;
constexpr uint16_t MAP_THIEVES_HIDEOUT_184 = 184;
constexpr uint16_t MAP_THIEVES_HIDEOUT_185 = 185;
constexpr uint16_t MAP_THIEVES_HIDEOUT_186 = 186;
constexpr uint16_t MAP_THIEVES_HIDEOUT_187 = 187;
constexpr uint16_t MAP_THIEVES_HIDEOUT_188 = 188;
constexpr uint16_t MAP_THIEVES_HIDEOUT_189 = 189;
constexpr uint16_t MAP_THIEVES_HIDEOUT_190 = 190;
constexpr uint16_t MAP_THIEVES_HIDEOUT_191 = 191;
constexpr uint16_t MAP_THIEVES_HIDEOUT_192 = 192;
constexpr uint16_t MAP_THIEVES_HIDEOUT_193 = 193;
constexpr uint16_t MAP_THIEVES_HIDEOUT_194 = 194;
constexpr uint16_t MAP_THIEVES_HIDEOUT_195 = 195;
constexpr uint16_t MAP_THIEVES_HIDEOUT_196 = 196;
constexpr uint16_t MAP_THIEVES_HIDEOUT_197 = 197;
constexpr uint16_t MAP_THIEVES_HIDEOUT_198 = 198;
constexpr uint16_t MAP_THIEVES_HIDEOUT_199 = 199;
constexpr uint16_t MAP_THIEVES_HIDEOUT_200 = 200;
constexpr uint16_t MAP_THIEVES_HIDEOUT_201 = 201;
constexpr uint16_t MAP_THIEVES_HIDEOUT_202 = 202;
constexpr uint16_t MAP_THIEVES_HIDEOUT_203 = 203;
constexpr uint16_t MAP_THIEVES_HIDEOUT_204 = 204;
constexpr uint16_t MAP_THIEVES_HIDEOUT_205 = 205;
constexpr uint16_t MAP_THIEVES_HIDEOUT_206 = 206;
constexpr uint16_t MAP_THIEVES_HIDEOUT_207 = 207;
constexpr uint16_t MAP_THIEVES_HIDEOUT_208 = 208;
constexpr uint16_t MAP_THIEVES_HIDEOUT_209 = 209;
constexpr uint16_t MAP_THIEVES_HIDEOUT_210 = 210;
constexpr uint16_t MAP_THIEVES_HIDEOUT_211 = 211;
constexpr uint16_t MAP_THIEVES_HIDEOUT_212 = 212;
constexpr uint16_t MAP_THIEVES_HIDEOUT_213 = 213;
constexpr uint16_t MAP_THIEVES_HIDEOUT_214 = 214;
constexpr uint16_t MAP_THIEVES_HIDEOUT_215 = 215;
constexpr uint16_t MAP_THIEVES_HIDEOUT_216 = 216;
constexpr uint16_t MAP_THIEVES_HIDEOUT_217 = 217;
constexpr uint16_t MAP_THIEVES_HIDEOUT_218 = 218;
constexpr uint16_t MAP_THIEVES_HIDEOUT_219 = 219;
constexpr uint16_t MAP_THIEVES_HIDEOUT_220 = 220;
constexpr uint16_t MAP_THIEVES_HIDEOUT_221 = 221;
constexpr uint16_t MAP_THIEVES_HIDEOUT_222 = 222;
constexpr uint16_t MAP_THIEVES_HIDEOUT_223 = 223;
constexpr uint16_t MAP_THIEVES_HIDEOUT_224 = 224;
constexpr uint16_t MAP_THIEVES_HIDEOUT_225 = 225;
constexpr uint16_t MAP_THIEVES_HIDEOUT_226 = 226;

constexpr uint16_t MAP_VERLA_MINES_227 = 227;
constexpr uint16_t MAP_VERLA_MINES_228 = 228;
constexpr uint16_t MAP_VERLA_MINES_229 = 229;
constexpr uint16_t MAP_VERLA_MINES_230 = 230;
constexpr uint16_t MAP_VERLA_MINES_231 = 231;
constexpr uint16_t MAP_VERLA_MINES_232 = 232;
constexpr uint16_t MAP_VERLA_MINES_233 = 233;
constexpr uint16_t MAP_VERLA_MINES_234 = 234;
constexpr uint16_t MAP_VERLA_MINES_235 = 235;
constexpr uint16_t MAP_VERLA_MINES_236 = 236;
constexpr uint16_t MAP_VERLA_MINES_237 = 237;
constexpr uint16_t MAP_VERLA_MINES_238 = 238;
constexpr uint16_t MAP_VERLA_MINES_239 = 239;
constexpr uint16_t MAP_VERLA_MINES_240 = 240;
constexpr uint16_t MAP_VERLA_MINES_241 = 241;
constexpr uint16_t MAP_VERLA_MINES_242 = 242;
constexpr uint16_t MAP_VERLA_MINES_243 = 243;
constexpr uint16_t MAP_VERLA_MINES_244 = 244;
constexpr uint16_t MAP_VERLA_MINES_245 = 245;
constexpr uint16_t MAP_VERLA_MINES_246 = 246;
constexpr uint16_t MAP_VERLA_MINES_247 = 247;
constexpr uint16_t MAP_VERLA_MINES_248 = 248;
constexpr uint16_t MAP_VERLA_MINES_249 = 249;
constexpr uint16_t MAP_VERLA_MINES_250 = 250;
constexpr uint16_t MAP_VERLA_MINES_251 = 251;
constexpr uint16_t MAP_VERLA_MINES_252 = 252;
constexpr uint16_t MAP_VERLA_MINES_253 = 253;
constexpr uint16_t MAP_VERLA_MINES_254 = 254;
constexpr uint16_t MAP_VERLA_MINES_255 = 255;
constexpr uint16_t MAP_VERLA_MINES_256 = 256;
constexpr uint16_t MAP_VERLA_MINES_257 = 257;
constexpr uint16_t MAP_VERLA_MINES_258 = 258;
constexpr uint16_t MAP_VERLA_MINES_259 = 259;
constexpr uint16_t MAP_VERLA_MINES_260 = 260;
constexpr uint16_t MAP_VERLA_MINES_261 = 261;
constexpr uint16_t MAP_VERLA_MINES_262 = 262;
constexpr uint16_t MAP_VERLA_MINES_263 = 263;
constexpr uint16_t MAP_VERLA_MINES_264 = 264;
constexpr uint16_t MAP_VERLA_MINES_265 = 265;
constexpr uint16_t MAP_VERLA_MINES_266 = 266;
constexpr uint16_t MAP_VERLA_MINES_267 = 267;
constexpr uint16_t MAP_VERLA_MINES_268 = 268;
constexpr uint16_t MAP_VERLA_MINES_269 = 269;

constexpr uint16_t MAP_DESTEL_WELL_270 = 270;
constexpr uint16_t MAP_DESTEL_WELL_271 = 271;
constexpr uint16_t MAP_DESTEL_WELL_272 = 272;
constexpr uint16_t MAP_DESTEL_WELL_273 = 273;
constexpr uint16_t MAP_DESTEL_WELL_274 = 274;
constexpr uint16_t MAP_DESTEL_WELL_275 = 275;
constexpr uint16_t MAP_DESTEL_WELL_276 = 276;
constexpr uint16_t MAP_DESTEL_WELL_277 = 277;
constexpr uint16_t MAP_DESTEL_WELL_278 = 278;
constexpr uint16_t MAP_DESTEL_WELL_279 = 279;
constexpr uint16_t MAP_DESTEL_WELL_280 = 280;
constexpr uint16_t MAP_DESTEL_WELL_281 = 281;
constexpr uint16_t MAP_DESTEL_WELL_282 = 282;
constexpr uint16_t MAP_DESTEL_WELL_283 = 283;
constexpr uint16_t MAP_DESTEL_WELL_284 = 284;
constexpr uint16_t MAP_DESTEL_WELL_285 = 285;
constexpr uint16_t MAP_DESTEL_WELL_286 = 286;
constexpr uint16_t MAP_DESTEL_WELL_287 = 287;
constexpr uint16_t MAP_DESTEL_WELL_288 = 288;
constexpr uint16_t MAP_DESTEL_WELL_289 = 289;
constexpr uint16_t MAP_DESTEL_WELL_290 = 290;

constexpr uint16_t MAP_LAKE_SHRINE_291 = 291;
constexpr uint16_t MAP_LAKE_SHRINE_292 = 292;
constexpr uint16_t MAP_LAKE_SHRINE_293 = 293;
constexpr uint16_t MAP_LAKE_SHRINE_294 = 294;
constexpr uint16_t MAP_LAKE_SHRINE_295 = 295;
constexpr uint16_t MAP_LAKE_SHRINE_296 = 296;
constexpr uint16_t MAP_LAKE_SHRINE_297 = 297;
constexpr uint16_t MAP_LAKE_SHRINE_298 = 298;
constexpr uint16_t MAP_LAKE_SHRINE_299 = 299;
constexpr uint16_t MAP_LAKE_SHRINE_300 = 300;
constexpr uint16_t MAP_LAKE_SHRINE_301 = 301;
constexpr uint16_t MAP_LAKE_SHRINE_302 = 302;
constexpr uint16_t MAP_LAKE_SHRINE_303 = 303;
constexpr uint16_t MAP_LAKE_SHRINE_304 = 304;
constexpr uint16_t MAP_LAKE_SHRINE_305 = 305;
constexpr uint16_t MAP_LAKE_SHRINE_306 = 306;
constexpr uint16_t MAP_LAKE_SHRINE_307 = 307;
constexpr uint16_t MAP_LAKE_SHRINE_308 = 308;
constexpr uint16_t MAP_LAKE_SHRINE_309 = 309;
constexpr uint16_t MAP_LAKE_SHRINE_310 = 310;
constexpr uint16_t MAP_LAKE_SHRINE_311 = 311;
constexpr uint16_t MAP_LAKE_SHRINE_312 = 312;
constexpr uint16_t MAP_LAKE_SHRINE_313 = 313;
constexpr uint16_t MAP_LAKE_SHRINE_314 = 314;
constexpr uint16_t MAP_LAKE_SHRINE_315 = 315;
constexpr uint16_t MAP_LAKE_SHRINE_316 = 316;
constexpr uint16_t MAP_LAKE_SHRINE_317 = 317;
constexpr uint16_t MAP_LAKE_SHRINE_318 = 318;
constexpr uint16_t MAP_LAKE_SHRINE_319 = 319;
constexpr uint16_t MAP_LAKE_SHRINE_320 = 320;
constexpr uint16_t MAP_LAKE_SHRINE_321 = 321;
constexpr uint16_t MAP_LAKE_SHRINE_322 = 322;
constexpr uint16_t MAP_LAKE_SHRINE_323 = 323;
constexpr uint16_t MAP_LAKE_SHRINE_324 = 324;
constexpr uint16_t MAP_LAKE_SHRINE_325 = 325;
constexpr uint16_t MAP_LAKE_SHRINE_326 = 326;
constexpr uint16_t MAP_LAKE_SHRINE_327 = 327;
constexpr uint16_t MAP_LAKE_SHRINE_328 = 328;
constexpr uint16_t MAP_LAKE_SHRINE_329 = 329;
constexpr uint16_t MAP_LAKE_SHRINE_330 = 330;
constexpr uint16_t MAP_LAKE_SHRINE_331 = 331;
constexpr uint16_t MAP_LAKE_SHRINE_332 = 332;
constexpr uint16_t MAP_LAKE_SHRINE_333 = 333;
constexpr uint16_t MAP_LAKE_SHRINE_334 = 334;
constexpr uint16_t MAP_LAKE_SHRINE_335 = 335;
constexpr uint16_t MAP_LAKE_SHRINE_336 = 336;
constexpr uint16_t MAP_LAKE_SHRINE_337 = 337;
constexpr uint16_t MAP_LAKE_SHRINE_338 = 338;
constexpr uint16_t MAP_LAKE_SHRINE_339 = 339;
constexpr uint16_t MAP_LAKE_SHRINE_340 = 340;
constexpr uint16_t MAP_LAKE_SHRINE_341 = 341;
constexpr uint16_t MAP_LAKE_SHRINE_342 = 342;
constexpr uint16_t MAP_LAKE_SHRINE_343 = 343;
constexpr uint16_t MAP_LAKE_SHRINE_344 = 344;
constexpr uint16_t MAP_LAKE_SHRINE_345 = 345;
constexpr uint16_t MAP_LAKE_SHRINE_346 = 346;
constexpr uint16_t MAP_LAKE_SHRINE_347 = 347;
constexpr uint16_t MAP_LAKE_SHRINE_348 = 348;
constexpr uint16_t MAP_LAKE_SHRINE_349 = 349;
constexpr uint16_t MAP_LAKE_SHRINE_350 = 350;
constexpr uint16_t MAP_LAKE_SHRINE_351 = 351;
constexpr uint16_t MAP_LAKE_SHRINE_352 = 352;
constexpr uint16_t MAP_LAKE_SHRINE_353 = 353;
constexpr uint16_t MAP_LAKE_SHRINE_354 = 354;

constexpr uint16_t MAP_KN_LABYRINTH_355 = 355;
constexpr uint16_t MAP_KN_LABYRINTH_356 = 356;
constexpr uint16_t MAP_KN_LABYRINTH_357 = 357;
constexpr uint16_t MAP_KN_LABYRINTH_358 = 358;
constexpr uint16_t MAP_KN_LABYRINTH_359 = 359;
constexpr uint16_t MAP_KN_LABYRINTH_360 = 360;
constexpr uint16_t MAP_KN_LABYRINTH_361 = 361;
constexpr uint16_t MAP_KN_LABYRINTH_362 = 362;
constexpr uint16_t MAP_KN_LABYRINTH_363 = 363;
constexpr uint16_t MAP_KN_LABYRINTH_364 = 364;
constexpr uint16_t MAP_KN_LABYRINTH_365 = 365;
constexpr uint16_t MAP_KN_LABYRINTH_366 = 366;
constexpr uint16_t MAP_KN_LABYRINTH_367 = 367;
constexpr uint16_t MAP_KN_LABYRINTH_368 = 368;
constexpr uint16_t MAP_KN_LABYRINTH_369 = 369;
constexpr uint16_t MAP_KN_LABYRINTH_370 = 370;
constexpr uint16_t MAP_KN_LABYRINTH_371 = 371;
constexpr uint16_t MAP_KN_LABYRINTH_372 = 372;
constexpr uint16_t MAP_KN_LABYRINTH_373 = 373;
constexpr uint16_t MAP_KN_LABYRINTH_374 = 374;
constexpr uint16_t MAP_KN_LABYRINTH_375 = 375;
constexpr uint16_t MAP_KN_LABYRINTH_376 = 376;
constexpr uint16_t MAP_KN_LABYRINTH_377 = 377;
constexpr uint16_t MAP_KN_LABYRINTH_378 = 378;
constexpr uint16_t MAP_KN_LABYRINTH_379 = 379;
constexpr uint16_t MAP_KN_LABYRINTH_380 = 380;
constexpr uint16_t MAP_KN_LABYRINTH_381 = 381;
constexpr uint16_t MAP_KN_LABYRINTH_382 = 382;
constexpr uint16_t MAP_KN_LABYRINTH_383 = 383;
constexpr uint16_t MAP_KN_LABYRINTH_384 = 384;
constexpr uint16_t MAP_KN_LABYRINTH_385 = 385;
constexpr uint16_t MAP_KN_LABYRINTH_386 = 386;
constexpr uint16_t MAP_KN_LABYRINTH_387 = 387;
constexpr uint16_t MAP_KN_LABYRINTH_388 = 388;
constexpr uint16_t MAP_KN_LABYRINTH_389 = 389;
constexpr uint16_t MAP_KN_LABYRINTH_390 = 390;
constexpr uint16_t MAP_KN_LABYRINTH_391 = 391;
constexpr uint16_t MAP_KN_LABYRINTH_392 = 392;
constexpr uint16_t MAP_KN_LABYRINTH_393 = 393;
constexpr uint16_t MAP_KN_LABYRINTH_394 = 394;
constexpr uint16_t MAP_KN_LABYRINTH_395 = 395;
constexpr uint16_t MAP_KN_LABYRINTH_396 = 396;
constexpr uint16_t MAP_KN_LABYRINTH_397 = 397;
constexpr uint16_t MAP_KN_LABYRINTH_398 = 398;
constexpr uint16_t MAP_KN_LABYRINTH_399 = 399;
constexpr uint16_t MAP_KN_LABYRINTH_400 = 400;
constexpr uint16_t MAP_KN_LABYRINTH_401 = 401;
constexpr uint16_t MAP_KN_LABYRINTH_402 = 402;
constexpr uint16_t MAP_KN_LABYRINTH_403 = 403;
constexpr uint16_t MAP_KN_LABYRINTH_404 = 404;
constexpr uint16_t MAP_KN_LABYRINTH_405 = 405;
constexpr uint16_t MAP_KN_LABYRINTH_406 = 406;
constexpr uint16_t MAP_KN_LABYRINTH_407 = 407;
constexpr uint16_t MAP_KN_LABYRINTH_408 = 408;
constexpr uint16_t MAP_KN_LABYRINTH_409 = 409;
constexpr uint16_t MAP_KN_LABYRINTH_410 = 410;
constexpr uint16_t MAP_KN_LABYRINTH_411 = 411;
constexpr uint16_t MAP_KN_LABYRINTH_412 = 412;
constexpr uint16_t MAP_KN_LABYRINTH_413 = 413;
constexpr uint16_t MAP_KN_LABYRINTH_414 = 414;
constexpr uint16_t MAP_KN_LABYRINTH_415 = 415;
constexpr uint16_t MAP_KN_LABYRINTH_416 = 416;
constexpr uint16_t MAP_KN_LABYRINTH_417 = 417;
constexpr uint16_t MAP_KN_LABYRINTH_418 = 418;
constexpr uint16_t MAP_KN_LABYRINTH_419 = 419;
constexpr uint16_t MAP_KN_LABYRINTH_420 = 420;
constexpr uint16_t MAP_STAIRWAY_TO_NOLE = 421;
constexpr uint16_t MAP_KN_LABYRINTH_422 = 422;
constexpr uint16_t MAP_KN_LABYRINTH_423 = 423;

constexpr uint16_t MAP_424 = 424;
constexpr uint16_t MAP_425 = 425;
constexpr uint16_t MAP_426 = 426;
constexpr uint16_t MAP_427 = 427;
constexpr uint16_t MAP_428 = 428;
constexpr uint16_t MAP_429 = 429;
constexpr uint16_t MAP_430 = 430;
constexpr uint16_t MAP_431 = 431;
constexpr uint16_t MAP_432 = 432;
constexpr uint16_t MAP_433 = 433;
constexpr uint16_t MAP_434 = 434;
constexpr uint16_t MAP_435 = 435;
constexpr uint16_t MAP_436 = 436;
constexpr uint16_t MAP_437 = 437;
constexpr uint16_t MAP_438 = 438;
constexpr uint16_t MAP_439 = 439;
constexpr uint16_t MAP_440 = 440;
constexpr uint16_t MAP_441 = 441;
constexpr uint16_t MAP_442 = 442;
constexpr uint16_t MAP_443 = 443;
constexpr uint16_t MAP_444 = 444;
constexpr uint16_t MAP_445 = 445;
constexpr uint16_t MAP_446 = 446;
constexpr uint16_t MAP_447 = 447;
constexpr uint16_t MAP_448 = 448;
constexpr uint16_t MAP_ROUTE_GUMI_RYUMA_BOULDER = 449;
constexpr uint16_t MAP_450 = 450;
constexpr uint16_t MAP_451 = 451;
constexpr uint16_t MAP_452 = 452;
constexpr uint16_t MAP_453 = 453;
constexpr uint16_t MAP_454 = 454;
constexpr uint16_t MAP_455 = 455;
constexpr uint16_t MAP_456 = 456;
constexpr uint16_t MAP_457 = 457;
constexpr uint16_t MAP_458 = 458;
constexpr uint16_t MAP_459 = 459;
constexpr uint16_t MAP_460 = 460;
constexpr uint16_t MAP_461 = 461;
constexpr uint16_t MAP_462 = 462;
constexpr uint16_t MAP_463 = 463;
constexpr uint16_t MAP_464 = 464;
constexpr uint16_t MAP_465 = 465;
constexpr uint16_t MAP_466 = 466;
constexpr uint16_t MAP_467 = 467;
constexpr uint16_t MAP_468 = 468;
constexpr uint16_t MAP_469 = 469;
constexpr uint16_t MAP_470 = 470;
constexpr uint16_t MAP_471 = 471;
constexpr uint16_t MAP_472 = 472;
constexpr uint16_t MAP_473 = 473;
constexpr uint16_t MAP_474 = 474;
constexpr uint16_t MAP_MIR_TOWER_EXTERIOR = 475;
constexpr uint16_t MAP_476 = 476;
constexpr uint16_t MAP_477 = 477;
constexpr uint16_t MAP_478 = 478;
constexpr uint16_t MAP_479 = 479;
constexpr uint16_t MAP_480 = 480;
constexpr uint16_t MAP_481 = 481;
constexpr uint16_t MAP_482 = 482;
constexpr uint16_t MAP_483 = 483;
constexpr uint16_t MAP_484 = 484;
constexpr uint16_t MAP_485 = 485;
constexpr uint16_t MAP_486 = 486;
constexpr uint16_t MAP_487 = 487;
constexpr uint16_t MAP_488 = 488;
constexpr uint16_t MAP_489 = 489;
constexpr uint16_t MAP_490 = 490;
constexpr uint16_t MAP_491 = 491;
constexpr uint16_t MAP_492 = 492;
constexpr uint16_t MAP_493 = 493;
constexpr uint16_t MAP_494 = 494;
constexpr uint16_t MAP_495 = 495;
constexpr uint16_t MAP_496 = 496;
constexpr uint16_t MAP_497 = 497;
constexpr uint16_t MAP_498 = 498;
constexpr uint16_t MAP_499 = 499;
constexpr uint16_t MAP_500 = 500;
constexpr uint16_t MAP_501 = 501;
constexpr uint16_t MAP_502 = 502;
constexpr uint16_t MAP_503 = 503;
constexpr uint16_t MAP_504 = 504;
constexpr uint16_t MAP_505 = 505;
constexpr uint16_t MAP_506 = 506;
constexpr uint16_t MAP_507 = 507;
constexpr uint16_t MAP_508 = 508;
constexpr uint16_t MAP_509 = 509;
constexpr uint16_t MAP_510 = 510;
constexpr uint16_t MAP_511 = 511;
constexpr uint16_t MAP_512 = 512;
constexpr uint16_t MAP_513 = 513;
constexpr uint16_t MAP_514 = 514;
constexpr uint16_t MAP_515 = 515;
constexpr uint16_t MAP_516 = 516;
constexpr uint16_t MAP_517 = 517;
constexpr uint16_t MAP_518 = 518;
constexpr uint16_t MAP_519 = 519;
constexpr uint16_t MAP_520 = 520;
constexpr uint16_t MAP_521 = 521;
constexpr uint16_t MAP_522 = 522;
constexpr uint16_t MAP_523 = 523;
constexpr uint16_t MAP_524 = 524;
constexpr uint16_t MAP_525 = 525;
constexpr uint16_t MAP_526 = 526;
constexpr uint16_t MAP_527 = 527;
constexpr uint16_t MAP_528 = 528;
constexpr uint16_t MAP_529 = 529;
constexpr uint16_t MAP_530 = 530;
constexpr uint16_t MAP_531 = 531;
constexpr uint16_t MAP_532 = 532;
constexpr uint16_t MAP_533 = 533;
constexpr uint16_t MAP_534 = 534;
constexpr uint16_t MAP_535 = 535;
constexpr uint16_t MAP_536 = 536;
constexpr uint16_t MAP_537 = 537;
constexpr uint16_t MAP_538 = 538;
constexpr uint16_t MAP_539 = 539;
constexpr uint16_t MAP_540 = 540;
constexpr uint16_t MAP_541 = 541;
constexpr uint16_t MAP_542 = 542;
constexpr uint16_t MAP_543 = 543;
constexpr uint16_t MAP_544 = 544;
constexpr uint16_t MAP_545 = 545;
constexpr uint16_t MAP_546 = 546;
constexpr uint16_t MAP_547 = 547;
constexpr uint16_t MAP_548 = 548;
constexpr uint16_t MAP_549 = 549;
constexpr uint16_t MAP_550 = 550;
constexpr uint16_t MAP_551 = 551;
constexpr uint16_t MAP_552 = 552;
constexpr uint16_t MAP_553 = 553;
constexpr uint16_t MAP_554 = 554;
constexpr uint16_t MAP_555 = 555;
constexpr uint16_t MAP_556 = 556;
constexpr uint16_t MAP_557 = 557;
constexpr uint16_t MAP_558 = 558;
constexpr uint16_t MAP_559 = 559;
constexpr uint16_t MAP_560 = 560;
constexpr uint16_t MAP_561 = 561;
constexpr uint16_t MAP_562 = 562;
constexpr uint16_t MAP_563 = 563;
constexpr uint16_t MAP_564 = 564;
constexpr uint16_t MAP_565 = 565;
constexpr uint16_t MAP_566 = 566;
constexpr uint16_t MAP_567 = 567;
constexpr uint16_t MAP_568 = 568;
constexpr uint16_t MAP_569 = 569;
constexpr uint16_t MAP_570 = 570;
constexpr uint16_t MAP_571 = 571;
constexpr uint16_t MAP_572 = 572;
constexpr uint16_t MAP_573 = 573;
constexpr uint16_t MAP_574 = 574;
constexpr uint16_t MAP_575 = 575;
constexpr uint16_t MAP_576 = 576;
constexpr uint16_t MAP_577 = 577;
constexpr uint16_t MAP_578 = 578;
constexpr uint16_t MAP_579 = 579;
constexpr uint16_t MAP_580 = 580;
constexpr uint16_t MAP_581 = 581;
constexpr uint16_t MAP_582 = 582;
constexpr uint16_t MAP_583 = 583;
constexpr uint16_t MAP_584 = 584;
constexpr uint16_t MAP_585 = 585;
constexpr uint16_t MAP_586 = 586;
constexpr uint16_t MAP_587 = 587;
constexpr uint16_t MAP_588 = 588;
constexpr uint16_t MAP_589 = 589;
constexpr uint16_t MAP_590 = 590;
constexpr uint16_t MAP_591 = 591;
constexpr uint16_t MAP_592 = 592;
constexpr uint16_t MAP_593 = 593;
constexpr uint16_t MAP_594 = 594;
constexpr uint16_t MAP_595 = 595;
constexpr uint16_t MAP_596 = 596;
constexpr uint16_t MAP_MASSAN_CHURCH = 597;
constexpr uint16_t MAP_598 = 598;
constexpr uint16_t MAP_599 = 599;
constexpr uint16_t MAP_600 = 600;
constexpr uint16_t MAP_601 = 601;
constexpr uint16_t MAP_602 = 602;
constexpr uint16_t MAP_603 = 603;
constexpr uint16_t MAP_604 = 604;
constexpr uint16_t MAP_605 = 605;
constexpr uint16_t MAP_606 = 606;
constexpr uint16_t MAP_GUMI_CHURCH = 607;
constexpr uint16_t MAP_608 = 608;
constexpr uint16_t MAP_609 = 609;
constexpr uint16_t MAP_610 = 610;
constexpr uint16_t MAP_611 = 611;
constexpr uint16_t MAP_612 = 612;
constexpr uint16_t MAP_RYUMA_CHURCH = 613;
constexpr uint16_t MAP_614 = 614;
constexpr uint16_t MAP_RYUMA_SHOP = 615;
constexpr uint16_t MAP_616 = 616;
constexpr uint16_t MAP_617 = 617;
constexpr uint16_t MAP_618 = 618;
constexpr uint16_t MAP_619 = 619;
constexpr uint16_t MAP_620 = 620;
constexpr uint16_t MAP_621 = 621;
constexpr uint16_t MAP_622 = 622;
constexpr uint16_t MAP_623 = 623;
constexpr uint16_t MAP_624 = 624;
constexpr uint16_t MAP_625 = 625;
constexpr uint16_t MAP_626 = 626;
constexpr uint16_t MAP_627 = 627;
constexpr uint16_t MAP_628 = 628;
constexpr uint16_t MAP_629 = 629;
constexpr uint16_t MAP_630 = 630;
constexpr uint16_t MAP_631 = 631;
constexpr uint16_t MAP_632 = 632;
constexpr uint16_t MAP_MERCATOR_ENTRANCE = 633;
constexpr uint16_t MAP_MERCATOR_ENTRANCE_VARIANT_1 = 634;
constexpr uint16_t MAP_MERCATOR_ENTRANCE_VARIANT_2 = 635;
constexpr uint16_t MAP_636 = 636;
constexpr uint16_t MAP_637 = 637;
constexpr uint16_t MAP_MERCATOR_FOUNTAIN = 638;
constexpr uint16_t MAP_MERCATOR_FOUNTAIN_DRIED_VARIANT = 639;
constexpr uint16_t MAP_640 = 640;
constexpr uint16_t MAP_641 = 641;
constexpr uint16_t MAP_642 = 642;
constexpr uint16_t MAP_MERCATOR_DOCKS_DARK_VARIANT = 643;
constexpr uint16_t MAP_MERCATOR_DOCKS_LIGHTHOUSE_FIXED_VARIANT = 644;
constexpr uint16_t MAP_645 = 645;
constexpr uint16_t MAP_646 = 646;
constexpr uint16_t MAP_647 = 647;
constexpr uint16_t MAP_648 = 648;
constexpr uint16_t MAP_649 = 649;
constexpr uint16_t MAP_650 = 650;
constexpr uint16_t MAP_651 = 651;
constexpr uint16_t MAP_652 = 652;
constexpr uint16_t MAP_653 = 653;
constexpr uint16_t MAP_654 = 654;
constexpr uint16_t MAP_655 = 655;
constexpr uint16_t MAP_656 = 656;
constexpr uint16_t MAP_657 = 657;
constexpr uint16_t MAP_658 = 658;
constexpr uint16_t MAP_659 = 659;
constexpr uint16_t MAP_660 = 660;
constexpr uint16_t MAP_661 = 661;
constexpr uint16_t MAP_662 = 662;
constexpr uint16_t MAP_663 = 663;
constexpr uint16_t MAP_664 = 664;
constexpr uint16_t MAP_665 = 665;
constexpr uint16_t MAP_666 = 666;
constexpr uint16_t MAP_667 = 667;
constexpr uint16_t MAP_668 = 668;
constexpr uint16_t MAP_669 = 669;
constexpr uint16_t MAP_670 = 670;
constexpr uint16_t MAP_671 = 671;
constexpr uint16_t MAP_672 = 672;
constexpr uint16_t MAP_673 = 673;
constexpr uint16_t MAP_674 = 674;
constexpr uint16_t MAP_675 = 675;
constexpr uint16_t MAP_676 = 676;
constexpr uint16_t MAP_677 = 677;
constexpr uint16_t MAP_678 = 678;
constexpr uint16_t MAP_679 = 679;
constexpr uint16_t MAP_680 = 680;
constexpr uint16_t MAP_681 = 681;
constexpr uint16_t MAP_682 = 682;
constexpr uint16_t MAP_683 = 683;
constexpr uint16_t MAP_684 = 684;
constexpr uint16_t MAP_685 = 685;
constexpr uint16_t MAP_686 = 686;
constexpr uint16_t MAP_687 = 687;
constexpr uint16_t MAP_688 = 688;
constexpr uint16_t MAP_689 = 689;
constexpr uint16_t MAP_690 = 690;
constexpr uint16_t MAP_691 = 691;
constexpr uint16_t MAP_692 = 692;
constexpr uint16_t MAP_693 = 693;
constexpr uint16_t MAP_694 = 694;
constexpr uint16_t MAP_MERCATOR_SPECIAL_SHOP = 695;
constexpr uint16_t MAP_MERCATOR_SPECIAL_SHOP_CONSUMABLE_VARIANT = 696;
constexpr uint16_t MAP_MERCATOR_SPECIAL_SHOP_VARIETY_VARIANT = 697;
constexpr uint16_t MAP_MERCATOR_SPECIAL_SHOP_BACKROOM = 698;
constexpr uint16_t MAP_MERCATOR_CHURCH = 699;
constexpr uint16_t MAP_MERCATOR_CHURCH_VARIANT = 700;
constexpr uint16_t MAP_701 = 701;
constexpr uint16_t MAP_702 = 702;
constexpr uint16_t MAP_703 = 703;
constexpr uint16_t MAP_704 = 704;
constexpr uint16_t MAP_705 = 705;
constexpr uint16_t MAP_706 = 706;
constexpr uint16_t MAP_707 = 707;
constexpr uint16_t MAP_708 = 708;
constexpr uint16_t MAP_709 = 709;
constexpr uint16_t MAP_710 = 710;
constexpr uint16_t MAP_711 = 711;
constexpr uint16_t MAP_712 = 712;
constexpr uint16_t MAP_713 = 713;
constexpr uint16_t MAP_714 = 714;
constexpr uint16_t MAP_715 = 715;
constexpr uint16_t MAP_716 = 716;
constexpr uint16_t MAP_717 = 717;
constexpr uint16_t MAP_718 = 718;
constexpr uint16_t MAP_719 = 719;
constexpr uint16_t MAP_720 = 720;
constexpr uint16_t MAP_VERLA_CHURCH = 721;
constexpr uint16_t MAP_722 = 722;
constexpr uint16_t MAP_723 = 723;
constexpr uint16_t MAP_724 = 724;
constexpr uint16_t MAP_725 = 725;
constexpr uint16_t MAP_726 = 726;
constexpr uint16_t MAP_727 = 727;
constexpr uint16_t MAP_728 = 728;
constexpr uint16_t MAP_DESTEL_INN = 729;
constexpr uint16_t MAP_730 = 730;
constexpr uint16_t MAP_731 = 731;
constexpr uint16_t MAP_732 = 732;
constexpr uint16_t MAP_733 = 733;
constexpr uint16_t MAP_734 = 734;
constexpr uint16_t MAP_735 = 735;
constexpr uint16_t MAP_DESTEL_CHURCH = 736;

constexpr uint16_t MAP_KAZALT_EXTERIOR = 737;
constexpr uint16_t MAP_KAZALT_738 = 738;
constexpr uint16_t MAP_KAZALT_739 = 739;
constexpr uint16_t MAP_KAZALT_740 = 740;
constexpr uint16_t MAP_KAZALT_741 = 741;
constexpr uint16_t MAP_KAZALT_742 = 742;
constexpr uint16_t MAP_KAZALT_743 = 743;
constexpr uint16_t MAP_KAZALT_744 = 744;
constexpr uint16_t MAP_KAZALT_745 = 745;
constexpr uint16_t MAP_KAZALT_746 = 746;
constexpr uint16_t MAP_KAZALT_SHOP = 747;
constexpr uint16_t MAP_KAZALT_748 = 748;
constexpr uint16_t MAP_KAZALT_CHURCH = 749;

constexpr uint16_t MAP_MIR_TOWER_750 = 750;
constexpr uint16_t MAP_MIR_TOWER_751 = 751;
constexpr uint16_t MAP_MIR_TOWER_752 = 752;
constexpr uint16_t MAP_MIR_TOWER_753 = 753;
constexpr uint16_t MAP_MIR_TOWER_754 = 754;
constexpr uint16_t MAP_MIR_TOWER_755 = 755;
constexpr uint16_t MAP_MIR_TOWER_756 = 756;
constexpr uint16_t MAP_MIR_TOWER_757 = 757;
constexpr uint16_t MAP_MIR_TOWER_758 = 758;
constexpr uint16_t MAP_MIR_TOWER_759 = 759;
constexpr uint16_t MAP_MIR_TOWER_760 = 760;
constexpr uint16_t MAP_MIR_TOWER_761 = 761;
constexpr uint16_t MAP_MIR_TOWER_762 = 762;
constexpr uint16_t MAP_MIR_TOWER_763 = 763;
constexpr uint16_t MAP_MIR_TOWER_764 = 764;
constexpr uint16_t MAP_MIR_TOWER_765 = 765;
constexpr uint16_t MAP_MIR_TOWER_766 = 766;
constexpr uint16_t MAP_MIR_TOWER_767 = 767;
constexpr uint16_t MAP_MIR_TOWER_768 = 768;
constexpr uint16_t MAP_MIR_TOWER_769 = 769;
constexpr uint16_t MAP_MIR_TOWER_770 = 770;
constexpr uint16_t MAP_MIR_TOWER_771 = 771;
constexpr uint16_t MAP_MIR_TOWER_772 = 772;
constexpr uint16_t MAP_MIR_TOWER_773 = 773;
constexpr uint16_t MAP_MIR_TOWER_774 = 774;
constexpr uint16_t MAP_MIR_TOWER_775 = 775;
constexpr uint16_t MAP_MIR_TOWER_776 = 776;
constexpr uint16_t MAP_MIR_TOWER_777 = 777;
constexpr uint16_t MAP_MIR_TOWER_778 = 778;
constexpr uint16_t MAP_MIR_TOWER_779 = 779;
constexpr uint16_t MAP_MIR_TOWER_780 = 780;
constexpr uint16_t MAP_MIR_TOWER_781 = 781;
constexpr uint16_t MAP_MIR_TOWER_782 = 782;
constexpr uint16_t MAP_MIR_TOWER_783 = 783;
constexpr uint16_t MAP_MIR_TOWER_BOSS_ARENA = 784;

constexpr uint16_t MAP_HELGAS_HUT_785 = 785;
constexpr uint16_t MAP_HELGAS_HUT_786 = 786;
constexpr uint16_t MAP_HELGAS_HUT_787 = 787;
constexpr uint16_t MAP_HELGAS_HUT_788 = 788;
constexpr uint16_t MAP_HELGAS_HUT_789 = 789;
constexpr uint16_t MAP_HELGAS_HUT_790 = 790;
constexpr uint16_t MAP_HELGAS_HUT_791 = 791;
constexpr uint16_t MAP_HELGAS_HUT_792 = 792;
constexpr uint16_t MAP_HELGAS_HUT_793 = 793;
constexpr uint16_t MAP_HELGAS_HUT_794 = 794;
constexpr uint16_t MAP_HELGAS_HUT_795 = 795;
constexpr uint16_t MAP_HELGAS_HUT_796 = 796;
constexpr uint16_t MAP_HELGAS_HUT_797 = 797;
constexpr uint16_t MAP_HELGAS_HUT_798 = 798;
constexpr uint16_t MAP_HELGAS_HUT_799 = 799;
constexpr uint16_t MAP_HELGAS_HUT_800 = 800;
constexpr uint16_t MAP_HELGAS_HUT_801 = 801;
constexpr uint16_t MAP_HELGAS_HUT_802 = 802;

constexpr uint16_t MAP_MASSAN_CAVE_803 = 803;
constexpr uint16_t MAP_MASSAN_CAVE_804 = 804;
constexpr uint16_t MAP_MASSAN_CAVE_805 = 805;
constexpr uint16_t MAP_MASSAN_CAVE_806 = 806;
constexpr uint16_t MAP_MASSAN_CAVE_807 = 807;

constexpr uint16_t MAP_TIBOR_808 = 808;
constexpr uint16_t MAP_TIBOR_809 = 809;
constexpr uint16_t MAP_TIBOR_810 = 810;
constexpr uint16_t MAP_TIBOR_811 = 811;
constexpr uint16_t MAP_TIBOR_812 = 812;
constexpr uint16_t MAP_TIBOR_813 = 813;
constexpr uint16_t MAP_TIBOR_814 = 814;
constexpr uint16_t MAP_TIBOR_815 = 815;