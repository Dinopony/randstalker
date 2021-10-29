#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"
#include "../exceptions.hpp"

void alter_item_order_in_menu(md::ROM& rom)
{
    std::vector<uint8_t> itemOrder = {
        ITEM_EKEEKE,        ITEM_RECORD_BOOK,
        ITEM_DAHL,          ITEM_RESTORATION,
        ITEM_GOLDEN_STATUE, ITEM_GAIA_STATUE,
        ITEM_DETOX_GRASS,   ITEM_MIND_REPAIR,
        ITEM_ANTI_PARALYZE, ITEM_ORACLE_STONE,
        ITEM_KEY,           ITEM_GARLIC,
        ITEM_LOGS,          ITEM_IDOL_STONE,
        ITEM_GOLA_EYE,      ITEM_GOLA_NAIL,
        ITEM_GOLA_HORN,     ITEM_GOLA_FANG,
        ITEM_DEATH_STATUE,  ITEM_STATUE_JYPTA,
        ITEM_SHORT_CAKE,    ITEM_PAWN_TICKET,
        ITEM_CASINO_TICKET, ITEM_LITHOGRAPH,
        ITEM_LANTERN,       ITEM_BELL,
        ITEM_SAFETY_PASS,   ITEM_ARMLET,
        ITEM_SUN_STONE,     ITEM_BUYER_CARD,
        ITEM_AXE_MAGIC,     ITEM_EINSTEIN_WHISTLE,
        ITEM_RED_JEWEL,     ITEM_PURPLE_JEWEL,
        ITEM_GREEN_JEWEL,   ITEM_BLUE_JEWEL,
        ITEM_YELLOW_JEWEL,  ITEM_SPELL_BOOK,
        ITEM_BLUE_RIBBON,   0xFF
    };

    constexpr uint32_t base_addr = 0x00D55C;
    for (int i = 0; base_addr + i < 0x00D584; ++i)
        rom.set_byte(base_addr + i, itemOrder[i]);
}


void rename_items(md::ROM& rom, const RandomizerOptions& options)
{
    std::vector<uint8_t> item_name_bytes;
    rom.data_chunk(0x29732, 0x29A0A, item_name_bytes);
    std::vector<std::vector<uint8_t>> item_names;

    // "Kazalt Jewel" mode is a specific mode when user asked for more jewels than we can provide individual items for.
    // In that case, we only use one generic jewel item type which can be obtained several times, and check against this
    // item's count instead of checking if every jewel type is owned at Kazalt teleporter
    bool kazalt_jewel_mode = (options.jewel_count() > MAX_INDIVIDUAL_JEWELS);

    // Read item names
    uint32_t addr = 0;
    while(true)
    {
        uint16_t stringSize = item_name_bytes[addr++];
        if(stringSize == 0xFF)
            break;

        // Clear Island Map name to make room for other names
        if(item_names.size() == ITEM_ISLAND_MAP)
            item_names.push_back(std::vector<uint8_t>({ 0x00 }));
        // Rename all default equipments with "None"
        else if(item_names.size() == ITEM_NO_SWORD || item_names.size() == ITEM_NO_ARMOR || item_names.size() == ITEM_NO_BOOTS)
            item_names.push_back({ 0x18, 0x33, 0x32, 0x29 });
        // Rename No52 into Green Jewel
        else if(item_names.size() == ITEM_GREEN_JEWEL && !kazalt_jewel_mode)
            item_names.push_back({ 0x11, 0x36, 0x29, 0x29, 0x32, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Rename Detox Book into Blue Jewel
        else if(item_names.size() == ITEM_BLUE_JEWEL && !kazalt_jewel_mode)
            item_names.push_back({ 0x0C, 0x30, 0x39, 0x29, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Rename AntiCurse Book into Yellow Jewel
        else if(item_names.size() == ITEM_YELLOW_JEWEL && !kazalt_jewel_mode)
            item_names.push_back({ 0x23, 0x29, 0x30, 0x30, 0x33, 0x3B, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 });
        // Clear "Purple Jewel" name to make room for other names since it's unused in Kazalt Jewel mode
        else if(item_names.size() == ITEM_PURPLE_JEWEL && kazalt_jewel_mode)
            item_names.push_back(std::vector<uint8_t>({ 0x00 }));
        // Rename "Red Jewel" into the more generic "Kazalt Jewel" in Kazalt Jewel mode
        else if(item_names.size() == ITEM_RED_JEWEL && kazalt_jewel_mode)
            item_names.push_back(std::vector<uint8_t>({ 0x15, 0x25, 0x3E, 0x25, 0x30, 0x38, 0x6A, 0x14, 0x29, 0x3B, 0x29, 0x30 }));
        // No specific treatment, just add it back as-is
        else
            item_names.push_back(std::vector<uint8_t>(item_name_bytes.begin() + addr, item_name_bytes.begin() + addr + stringSize));

        addr += stringSize;
    }

    constexpr uint16_t initialSize = 0x29A0A - 0x29732;

    item_name_bytes.clear();
    for(const std::vector<uint8_t>& itemName : item_names)
    {
        item_name_bytes.push_back((uint8_t)itemName.size());
        item_name_bytes.insert(item_name_bytes.end(), itemName.begin(), itemName.end());
    }
    item_name_bytes.push_back(0xFF);

    if(item_name_bytes.size() > initialSize)
        throw new RandomizerException("Item names size is above initial game size");
    rom.set_bytes(0x29732, item_name_bytes);
}

void change_hud_color(md::ROM& rom, const RandomizerOptions& options)
{
    // 0x824 is the default purple color from the original game
    uint16_t color = 0x0824;

    std::string hud_color = options.hud_color();
    tools::to_lower(hud_color);

    if (hud_color == "red")              color = 0x228;
    else if (hud_color == "darkred")     color = 0x226;
    else if (hud_color == "green")       color = 0x262;
    else if (hud_color == "blue")        color = 0x842;
    else if (hud_color == "brown")       color = 0x248;
    else if (hud_color == "darkpurple")  color = 0x424;
    else if (hud_color == "darkgray")    color = 0x222;
    else if (hud_color == "gray")        color = 0x444;
    else if (hud_color == "lightgray")   color = 0x666;

    rom.set_word(0xF6D0, color);
    rom.set_word(0xFB36, color);
    rom.set_word(0x903C, color);
//    rom.set_word(0x9020, color);
}

void shorten_mir_death_cutscene(md::ROM& rom)
{
    // Cut the cutscene script
    rom.set_word(0x2872C, 0xE646);
    rom.set_word(0x2873C, 0xE64B);
}

void shorten_mir_cutscene_after_lake_shrine(md::ROM& rom)
{
    // Cut the cutscene script
    rom.set_word(0x28A44, 0xE739);
}

void alter_credits(md::ROM& rom)
{
    constexpr uint32_t credits_text_base_addr = 0x9ED1A;

    // Change "LANDSTALKER" to "RANDSTALKER"
    rom.set_byte(credits_text_base_addr + 0x2, 0x13);

    rom.set_bytes(credits_text_base_addr + 0x14, { 
        0x27, 0x1C, 0x29, 0x1F, 0x2E, 0x2F, 0x1C, 0x27, 0x26, 0x20, 0x2D, 0x80, // "landstalker "
        0x2D, 0x1C, 0x29, 0x1F, 0x2A, 0x28, 0x24, 0x35, 0x20, 0x2D              // "randomizer"
    });

    // Widen the space between the end of the cast and the beginning of the rando staff
    rom.set_byte(credits_text_base_addr + 0x5C, 0x0F);

    rom.set_bytes(credits_text_base_addr + 0x75, { 
        // RANDOMIZER
        0x08, 0xFF, 0x82, 
        0x80, 0x13, 0x80, 0x02, 0x80, 0x0F, 0x80, 0x05, 0x80, 0x10, 0x80, 0x0E, 0x80, 0x0A, 0x80, 0x1B, 0x80, 0x06, 0x80, 0x13, 0x80, 0x00, 
        // developed by
        0x04, 0xFF, 0x81, 
        0x1F, 0x20, 0x31, 0x20, 0x27, 0x2A, 0x2B, 0x20, 0x1F, 0x80, 0x1D, 0x34, 0x00,
        // DINOPONY
        0x03, 0xFF,
        0x05, 0x0A, 0x0F, 0x10, 0x11, 0x10, 0x0F, 0x1A, 0x00,
        // WIZ
        0x08, 0xFF,
        0x18, 0x0A, 0x1B, 0x00,
        // testing
        0x04, 0xFF, 0x81,
        0x2F, 0x20, 0x2E, 0x2F, 0x24, 0x29, 0x22, 0x00,
        // SAGAZ
        0x03, 0xFF,
        0x14, 0x02, 0x08, 0x02, 0x1B, 0x00,
        // STIK
        0x03, 0xFF,
        0x14, 0x15, 0x0A, 0x0C, 0x00,
        // WIZ
        0x03, 0xFF,
        0x18, 0x0A, 0x1B, 0x00,
        // LANDRYLE
        0x08, 0xFF,
        0x0D, 0x02, 0x0F, 0x05, 0x13, 0x1A, 0x0D, 0x06, 0x00,
        // website by
        0x04, 0xFF, 0x81,
        0x32, 0x20, 0x1D, 0x2E, 0x24, 0x2F, 0x20, 0x80, 0x1D, 0x34, 0x00,
        // DILANDAU
        0x08, 0xFF,
        0x05, 0x0A, 0x0D, 0x02, 0x0F, 0x05, 0x02, 0x16, 0x00
    });

    std::vector<uint8_t> rest;
    rom.data_chunk(credits_text_base_addr + 0x1D2, credits_text_base_addr + 0x929, rest);
    rom.set_bytes(credits_text_base_addr + 0xF5, rest);
    for(uint32_t addr = credits_text_base_addr + 0xF5 + (uint32_t)rest.size() ; addr <= credits_text_base_addr + 0x929 ; ++addr)
        rom.set_byte(addr, 0x00);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_quality_of_life(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // UI changes
    alter_item_order_in_menu(rom);
    rename_items(rom, options);
    change_hud_color(rom, options);
    alter_credits(rom);

    // Other QoL changes
    shorten_mir_death_cutscene(rom);
    shorten_mir_cutscene_after_lake_shrine(rom);
}
