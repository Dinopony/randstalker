#include <md_tools.hpp>

#include "../../logic_model/world_logic.hpp"
#include "../../logic_model/world_path.hpp"
#include "../../world_model/world.hpp"
#include "../../world_model/map.hpp"
#include "../../constants/item_codes.hpp"
#include "../../constants/map_codes.hpp"
#include "../../constants/offsets.hpp"
#include "../../constants/values.hpp"
#include "../../exceptions.hpp"

#include "assets/blue_jewel.bin.hxx"
#include "assets/yellow_jewel.bin.hxx"
#include "assets/green_jewel.bin.hxx"

static void add_jewel_check_for_kazalt_teleporter(md::ROM& rom, uint8_t jewel_count)
{
    // ----------- Rejection textbox handling ------------
    md::Code func_reject_kazalt_tp;

    func_reject_kazalt_tp.jsr(0x22EE8); // open textbox
    func_reject_kazalt_tp.movew(0x22, reg_D0);
    func_reject_kazalt_tp.jsr(0x28FD8); // display text 
    func_reject_kazalt_tp.jsr(0x22EA0); // close textbox
    func_reject_kazalt_tp.rts();

    uint32_t func_reject_kazalt_tp_addr = rom.inject_code(func_reject_kazalt_tp);

    // ----------- Jewel checks handling ------------
    md::Code proc_handle_jewels_check;

    if(jewel_count > MAX_INDIVIDUAL_JEWELS)
    {
        proc_handle_jewels_check.movem_to_stack({reg_D1},{});
        proc_handle_jewels_check.moveb(addr_(0xFF1054), reg_D1);
        proc_handle_jewels_check.andib(0x0F, reg_D1);
        proc_handle_jewels_check.cmpib(jewel_count, reg_D1); // Test if red jewel is owned
        proc_handle_jewels_check.movem_from_stack({reg_D1},{});
        proc_handle_jewels_check.bgt(3);
            proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
            proc_handle_jewels_check.rts();
    }
    else
    {
        if(jewel_count >= 1)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1054)); // Test if red jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(jewel_count >= 2)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1055)); // Test if purple jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(jewel_count >= 3)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF105A)); // Test if green jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(jewel_count >= 4)
        {
            proc_handle_jewels_check.btst(0x5, addr_(0xFF1050)); // Test if blue jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
        if(jewel_count >= 5)
        {
            proc_handle_jewels_check.btst(0x1, addr_(0xFF1051)); // Test if yellow jewel is owned
            proc_handle_jewels_check.bne(3);
                proc_handle_jewels_check.jsr(func_reject_kazalt_tp_addr);
                proc_handle_jewels_check.rts();
        }
    }
    proc_handle_jewels_check.moveq(0x7, reg_D0);
    proc_handle_jewels_check.jsr(0xE110);  // "func_teleport_kazalt"
    proc_handle_jewels_check.jmp(0x62FA);

    uint32_t handle_jewels_addr = rom.inject_code(proc_handle_jewels_check);

    // This adds the jewels as a requirement for the Kazalt teleporter to work correctly
    rom.set_code(0x62F4, md::Code().jmp(handle_jewels_addr));
}

static void rename_jewels(md::ROM& rom, World& world, uint8_t jewel_count)
{
    std::vector<uint8_t> item_name_bytes;
    rom.data_chunk(0x29732, 0x29A0A, item_name_bytes);
    std::vector<std::vector<uint8_t>> item_names;

    // "Kazalt Jewel" mode is a specific mode when user asked for more jewels than we can provide individual items for.
    // In that case, we only use one generic jewel item type which can be obtained several times, and check against this
    // item's count instead of checking if every jewel type is owned at Kazalt teleporter
    bool kazalt_jewel_mode = (jewel_count > MAX_INDIVIDUAL_JEWELS);
    if(kazalt_jewel_mode)
    {
        Item* red_jewel = world.item(ITEM_RED_JEWEL);
        red_jewel->name("Kazalt Jewel");
        red_jewel->allowed_on_ground(false);
        red_jewel->max_quantity(jewel_count);
    }

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

    constexpr uint16_t initialSize = offsets::ITEM_NAMES_TABLE_END - offsets::ITEM_NAMES_TABLE;
    
    item_name_bytes.clear();
    for(const std::vector<uint8_t>& itemName : item_names)
    {
        item_name_bytes.push_back((uint8_t)itemName.size());
        item_name_bytes.insert(item_name_bytes.end(), itemName.begin(), itemName.end());
    }
    item_name_bytes.push_back(0xFF);

    if(item_name_bytes.size() > initialSize)
        throw new RandomizerException("Item names size is above initial game size");
    rom.set_bytes(offsets::ITEM_NAMES_TABLE, item_name_bytes);
}

static void remove_books_replaced_by_jewels(md::ROM& rom, World& world, uint8_t jewel_count)
{
    if(jewel_count > 3 && jewel_count <= MAX_INDIVIDUAL_JEWELS)
    {
        // Remove jewels replaced from book IDs from priest stands
        world.map(MAP_MASSAN_CHURCH)->remove_entity(3);
        world.map(MAP_MASSAN_CHURCH)->remove_entity(2);
        world.map(MAP_GUMI_CHURCH)->remove_entity(2);
        world.map(MAP_GUMI_CHURCH)->remove_entity(1);
        world.map(MAP_RYUMA_CHURCH)->remove_entity(4);
        world.map(MAP_RYUMA_CHURCH)->remove_entity(3);
        world.map(MAP_MERCATOR_CHURCH_VARIANT)->remove_entity(3);
        world.map(MAP_MERCATOR_CHURCH_VARIANT)->remove_entity(2);
        world.map(MAP_VERLA_CHURCH)->remove_entity(3);
        world.map(MAP_VERLA_CHURCH)->remove_entity(2);
        world.map(MAP_DESTEL_CHURCH)->remove_entity(3);
        world.map(MAP_DESTEL_CHURCH)->remove_entity(2);
        world.map(MAP_KAZALT_CHURCH)->remove_entity(3);
        world.map(MAP_KAZALT_CHURCH)->remove_entity(2);

        // Make the Awakening Book (the only one remaining in churches) heal all status conditions
        rom.set_code(0x24F6C, md::Code().nop(6));
        rom.set_code(0x24FB8, md::Code().moveb(0xFF, reg_D0));

        // Change the behavior of AntiCurse and Detox books (now Yellow and Blue jewels) in shops
        rom.set_byte(0x24C40, 0x40);
        rom.set_byte(0x24C58, 0x40);
    }
}

/**
 * The randomizer has an option to handle more jewels than the good old Red and Purple ones.
 * It can handle up to 9 jewels, but we only can afford having 5 unique jewel items.
 * This means there are two modes:
 *      - Unique jewels mode (1-5 jewels): each jewel has its own ID, name and sprite
 *      - Kazalt jewels mode (6+ jewels): jewels are one generic item that can be stacked up to 9 times
 *
 * This function handles the "unique jewels mode" by replacing useless items (priest books), injecting
 * new sprites and taking care of everything for this to happen.
 */
static void add_additional_jewel_sprites(md::ROM& rom, uint8_t jewel_count)
{
    // If we are in "Kazalt jewel" mode, don't do anything
    if(jewel_count > MAX_INDIVIDUAL_JEWELS)
        return;
    
    if(jewel_count >= 3)
    {
        // Add a sprite for green jewel and make the item use it
        uint32_t green_jewel_sprite_addr = rom.inject_bytes(GREEN_JEWEL_SPRITE, GREEN_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_GREEN_JEWEL * 0x4), green_jewel_sprite_addr); // 0x121648
    }
    if(jewel_count >= 4)
    {
        // Add a sprite for blue jewel and make the item use it
        uint32_t blue_jewel_sprite_addr = rom.inject_bytes(BLUE_JEWEL_SPRITE, BLUE_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_BLUE_JEWEL * 0x4), blue_jewel_sprite_addr);
    }
    if(jewel_count >= 5)
    {
        // Add a sprite for green jewel and make the item use it
        uint32_t yellow_jewel_sprite_addr = rom.inject_bytes(YELLOW_JEWEL_SPRITE, YELLOW_JEWEL_SPRITE_SIZE);
        rom.set_long(offsets::ITEM_SPRITES_TABLE + (ITEM_YELLOW_JEWEL * 0x4), yellow_jewel_sprite_addr);
    }
}

static void patch_logic_with_jewels(WorldLogic& logic, World& world, uint8_t jewel_count)
{
    // Determine the list of required jewels to go from King Nole's Cave to Kazalt depending on settings
    WorldPath* path_to_kazalt = logic.path("king_nole_cave", "kazalt");
    if(jewel_count > MAX_INDIVIDUAL_JEWELS)
    {
        for(int i=0; i<jewel_count ; ++i)
            path_to_kazalt->add_required_item(world.item(ITEM_RED_JEWEL));
    }
    else if(jewel_count >= 1)
    {
        path_to_kazalt->add_required_item(world.item(ITEM_RED_JEWEL));
        if(jewel_count >= 2)
            path_to_kazalt->add_required_item(world.item(ITEM_PURPLE_JEWEL));
        if(jewel_count >= 3)
            path_to_kazalt->add_required_item(world.item(ITEM_GREEN_JEWEL));
        if(jewel_count >= 4)
            path_to_kazalt->add_required_item(world.item(ITEM_BLUE_JEWEL));
        if(jewel_count >= 5)
            path_to_kazalt->add_required_item(world.item(ITEM_YELLOW_JEWEL));
    }
}

void handle_additional_jewels(md::ROM& rom, World& world, WorldLogic& logic, uint8_t jewel_count)
{
    add_jewel_check_for_kazalt_teleporter(rom, jewel_count);
    rename_jewels(rom, world, jewel_count);
    remove_books_replaced_by_jewels(rom, world, jewel_count);
    add_additional_jewel_sprites(rom, jewel_count);
    patch_logic_with_jewels(logic, world, jewel_count);
}