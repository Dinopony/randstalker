#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"

#include "../world_model/world.hpp"
#include "../world_model/spawn_location.hpp"
#include "../world_model/item.hpp"

#include "../constants/item_codes.hpp"

static void handle_spawn_position(md::ROM& rom, const World& world)
{
    rom.set_word(0x0027F4, world.spawn_location().map_id());
    rom.set_byte(0x0027FD, world.spawn_location().position_x());
    rom.set_byte(0x002805, world.spawn_location().position_y());
}

static void handle_custom_starting_life(md::ROM& rom, const World& world)
{
    uint8_t starting_life = world.starting_life();
    rom.set_byte(0x0027B4, starting_life-1);
    rom.set_byte(0x0027BC, starting_life-1);
}

static std::vector<uint8_t> build_flag_array(const World& world, bool add_ingame_tracker)
{
    std::vector<uint8_t> flag_array;
    flag_array.resize(0x60, 0x00);
    
    // Apply starting flags
    for(const Flag& flag : world.starting_flags())
        flag_array[flag.byte] |= (1 << flag.bit); 

    // Apply starting inventory
    for(uint8_t item_id=0 ; item_id < ITEM_GOLDS_START ; item_id += 0x2)
    {
        uint8_t inventory_flag_value = 0x00;

        uint8_t lsh_quantity = world.item(item_id)->starting_quantity();
        if(lsh_quantity)
            inventory_flag_value |= (lsh_quantity+1) & 0x0F;

        uint8_t msh_quantity = world.item(item_id+1)->starting_quantity();
        if(msh_quantity)
            inventory_flag_value |= ((msh_quantity+1) & 0x0F) << 4;

        flag_array[0x40+(item_id/2)] = inventory_flag_value;
    }

    if(add_ingame_tracker)
    {
        // The ingame-tracker consists in putting in "grayed-out" key items in the inventory,
        // as if they were already obtained but lost (like lithograph in OG)
        flag_array[0x4B] |= 0x10;
        flag_array[0x4C] |= 0x10;
        flag_array[0x4D] |= 0x11;
        flag_array[0x4F] |= 0x11;
        flag_array[0x50] |= 0x01;
        flag_array[0x55] |= 0x10;
        flag_array[0x57] |= 0x10;
        flag_array[0x58] |= 0x10;
        flag_array[0x59] |= 0x11;
        flag_array[0x5B] |= 0x10;
        flag_array[0x5C] |= 0x11;
    }

    return flag_array;
}

static void inject_func_init_game(md::ROM& rom, const World& world, bool add_ingame_item_tracker)
{
    std::vector<uint8_t> flag_array = build_flag_array(world, add_ingame_item_tracker);

    // ------- Inject flags init function ---------
    // Init function used to set story flags to specific values at the very beginning of the game, opening some usually closed paths
    // and removing some useless cutscenes (considering them as "already seen").

    md::Code func_init_game;

    // Init all flags with the contents of flag_array
    for(int i=0 ; i<flag_array.size() ; i+=0x2)
    {
        uint32_t value = (static_cast<uint16_t>(flag_array[i]) << 8) + static_cast<uint16_t>(flag_array[i+1]);
        if(value)
            func_init_game.movew(value, addr_(0xFF1000+i));
    }

    // Set the orientation byte of Nigel depending on spawn location on game start
    func_init_game.moveb(world.spawn_location().orientation(), addr_(0xFF5404));
    // Set the appropriate starting golds
    func_init_game.movew(world.starting_golds(), addr_(0xFF120E));

    func_init_game.rts();

    uint32_t func_init_game_addr = rom.inject_code(func_init_game);
    
    // ------- Remove no music flag ---------
    // Replace the bitset of the no music flag by a jump to 'func_init_game'
    // 0x002700:
        // Before: 	[08F9] bset 3 -> $FF1027
        // After:	[4EB9] jsr $1FFAD0 ; [4E71] nop
    rom.set_code(0x002700, md::Code().jsr(func_init_game_addr).nop());
}

static void remove_cutscene_flag_on_game_start(md::ROM& rom)
{
    // ------- Remove cutscene flag (no input allowed) ---------
    // Usually, when starting a new game, it is automatically put into "cutscene mode" to 
    // let the intro roll without allowing the player to move or pause, or do anything at all. 
    // We need to remove that cutscene flag to enable the player actually playing the game.
    // 0x00281A:
        // Before:	[33FC] move.w 0x00FE -> $FF12DE
        // After:	[4E71] nop (4 times)
    rom.set_code(0x281A, md::Code().nop(4));
}

void disable_region_check(md::ROM& rom)
{
    // Before : jsr $A0A0C | After : nop nop nop
    rom.set_code(0x506, md::Code().nop(3));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

void patch_game_init(md::ROM& rom, const World& world, bool add_ingame_tracker)
{
    // Inject a function to setup flags and properties on game init
    inject_func_init_game(rom, world, add_ingame_tracker);

    // Handle a few more properties
    handle_spawn_position(rom, world);
    handle_custom_starting_life(rom, world);

    // Remove a blocking cutscene flag to be allowed to play
    remove_cutscene_flag_on_game_start(rom);

    // Remove node check on game boot
    disable_region_check(rom);
}
