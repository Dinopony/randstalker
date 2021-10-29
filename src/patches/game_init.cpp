#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"

static void setup_story_flags(const RandomizerOptions& options, const World& world, std::vector<uint8_t>& out_flag_array)
{
    out_flag_array[0x00] = 0xB1;
    out_flag_array[0x02] = 0x95;
    out_flag_array[0x03] = 0xE0;
    out_flag_array[0x04] = 0xD1;
    out_flag_array[0x05] = 0x60;
    out_flag_array[0x06] = 0x7C;
    out_flag_array[0x07] = 0xB4;
    out_flag_array[0x08] = 0xFE;
    out_flag_array[0x09] = 0x7E;
    out_flag_array[0x12] = 0x1E;
    out_flag_array[0x13] = 0x43;
    out_flag_array[0x14] = 0x81;
    out_flag_array[0x17] = 0x48;
    out_flag_array[0x20] = 0x70;
    out_flag_array[0x26] = 0x80;
    out_flag_array[0x27] = 0x22;
    out_flag_array[0x28] = 0xE0;
    out_flag_array[0x29] = 0x40;
    out_flag_array[0x2A] = 0x81;
    out_flag_array[0x2B] = 0x82;

    // Clear Verla soldiers if spawning in Verla
    if(world.active_spawn_location()->id() == "verla")
        out_flag_array[0x26] += 0x18;

    // Mark the boulder as already removed at game start
    if(options.remove_gumi_boulder())
        out_flag_array[0x02] += 0x40;
}

static void setup_starting_items(const World& world, std::vector<uint8_t>& out_flag_array)
{
    // Set starting items
    for(uint8_t item_id=0 ; item_id < ITEM_GOLDS_START ; item_id += 0x2)
    {
        uint8_t inventory_flag_value = 0x00;

        uint8_t lsh_quantity = world.item(item_id)->starting_quantity();
        if(lsh_quantity)
            inventory_flag_value |= (lsh_quantity+1) & 0x0F;

        uint8_t msh_quantity = world.item(item_id+1)->starting_quantity();
        if(msh_quantity)
            inventory_flag_value |= ((msh_quantity+1) & 0x0F) << 4;

        out_flag_array[0x40+(item_id/2)] = inventory_flag_value;
    }
}

static void setup_ingame_tracker(std::vector<uint8_t>& out_flag_array)
{
    // The ingame-tracker consists in putting in "greyed-out" key items in the inventory,
    // as if they were already obtained but lost (like lithograph in OG)
    out_flag_array[0x4B] |= 0x10;
    out_flag_array[0x4C] |= 0x10;
    out_flag_array[0x4D] |= 0x11;
    out_flag_array[0x4F] |= 0x11;
    out_flag_array[0x50] |= 0x01;
    out_flag_array[0x55] |= 0x10;
    out_flag_array[0x57] |= 0x10;
    out_flag_array[0x58] |= 0x10;
    out_flag_array[0x59] |= 0x11;
    out_flag_array[0x5B] |= 0x10;
    out_flag_array[0x5C] |= 0x11;
}

static void handle_spawn_position(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    rom.set_word(0x0027F4, world.active_spawn_location()->map_id());
    rom.set_byte(0x0027FD, world.active_spawn_location()->position_x());
    rom.set_byte(0x002805, world.active_spawn_location()->position_y());
}

static void handle_custom_starting_life(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    uint8_t starting_life = options.starting_life();
    if(!starting_life)
        starting_life = world.active_spawn_location()->starting_life();
    rom.set_byte(0x0027B4, starting_life-1);
    rom.set_byte(0x0027BC, starting_life-1);
}

static void inject_func_init_game(md::ROM& rom, const RandomizerOptions& options, const World& world, const std::vector<uint8_t>& flag_array)
{
    // ------- Inject flags init function ---------
    // Init function used to set story flags to specific values at the very beginning of the game, opening some usually closed paths
    // and removing some useless cutscenes (considering them as "already seen").

    md::Code func_init_game;

    // Init all flags with the contents of flag_array
    for(int i=0 ; i<0x60 ; i+=0x2)
    {
        uint32_t value = (static_cast<uint16_t>(flag_array[i]) << 8) + static_cast<uint16_t>(flag_array[i+1]);
        if(value)
            func_init_game.movew(value, addr_(0xFF1000+i));
    }

    // Set the orientation byte of Nigel depending on spawn location on game start
    func_init_game.moveb(world.active_spawn_location()->orientation(), addr_(0xFF5404));
    // Set the appropriate starting golds
    func_init_game.movew(options.starting_gold(), addr_(0xFF120E));

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

void patch_game_init(md::ROM& rom, const RandomizerOptions& options, const World& world)
{
    // Inject a function to setup flags and properties on game init
    std::vector<uint8_t> flag_array;
    flag_array.resize(0x60, 0x00); 
    
    setup_story_flags(options, world, flag_array);
    setup_starting_items(world, flag_array);
    if (options.add_ingame_item_tracker())
    {
        setup_ingame_tracker(flag_array);
    }

    inject_func_init_game(rom, options, world, flag_array);

    // Handle a few more properties
    handle_spawn_position(rom, options, world);
    handle_custom_starting_life(rom, options, world);

    // Remove a blocking cutscene flag to be allowed to play
    remove_cutscene_flag_on_game_start(rom);

    // Remove region check on game boot
    disable_region_check(rom);
}
