#include "../tools/megadrive/rom.hpp"
#include "../tools/megadrive/code.hpp"
#include "../randomizer_options.hpp"
#include "../world.hpp"
#include "../model/spawn_location.hpp"

// Flag 1000
constexpr uint8_t FLAG_NIGEL_AWOKEN_FROM_KNC_WATERFALL = 0x80;
constexpr uint8_t FLAG_INTRO_FINISHED = 0x20;
constexpr uint8_t FLAG_KEYDOOR_OPENED_IN_KNC = 0x10;
constexpr uint8_t FLAG_FARA_KIDNAPPED = 0x01;

// Flag 1002
constexpr uint8_t FLAG_TALKED_WITH_HIDDEN_VILLAGER_RYUMA = 0x80;
constexpr uint8_t FLAG_SPIED_ON_GUMI_RITUAL = 0x10;
constexpr uint8_t FLAG_BRIDGE_TO_GUMI_REPAIRED = 0x04;
constexpr uint8_t FLAG_MASSAN_VILLAGER_WARNED_BRIDGE_IS_BROKEN = 0x01;

// Flag 1003
constexpr uint8_t FLAG_FIRST_ENCOUNTER_WITH_DUKE = 0x80;
constexpr uint8_t FLAG_SAW_FRIDAY_CUTSCENE_AT_MADAME_YARD = 0x40;
constexpr uint8_t FLAG_ARTHUR_GAVE_ACCESS_TO_CASTLE = 0x20;

// Flag 1004
constexpr uint8_t FLAG_TALKED_WITH_MERCATOR_GUARDS_ASKING_GOLDS = 0x80;
constexpr uint8_t FLAG_SAW_KAYLA_RAFT_CUTSCENE_AT_RYUMA = 0x40;
constexpr uint8_t FLAG_ALLOWED_TO_GO_IN_THIEVES_HIDEOUT = 0x01;

// Flag 1005
constexpr uint8_t FLAG_TALKED_WITH_MADAME_YARD_GIRL_HINTING_FORTUNE_TELLER = 0x40;
constexpr uint8_t FLAG_SAW_ARTHUR_AT_CASTLE_BACKDOOR = 0x20;

constexpr uint8_t FLAG_ASSISTED_TO_MERCATOR_BANQUET = 0x10;
constexpr uint8_t FLAG_TALKED_WITH_KITCHEN_WOMAN_BEFORE_BANQUET = 0x08;
constexpr uint8_t FLAG_SERVANT_BORUGHT_US_TO_BEDROOM = 0x04;
constexpr uint8_t FLAG_DUKE_LEFT_ROOM_AFTER_FIRST_ENCOUNTER = 0x02;

constexpr uint8_t FLAG_COMPLETED_MERCATOR_JARS_SIDEQUEST_TWO_TIMES = 0x40;
constexpr uint8_t FLAG_SAW_TRAPPED_KAYLA_CUTSCENE_IN_DUNGEON = 0x02;
constexpr uint8_t FLAG_THREW_IN_DUNGEON_BY_DUKE = 0x01;

constexpr uint8_t FLAG_SAW_KAYLA_LITHOGRAPH_CUTSCENE = 0x80;
constexpr uint8_t FLAG_SLEPT_AT_MERCATOR_AFTER_BANQUET = 0x01;

constexpr uint8_t FLAG_TOOK_PURPLE_JEWEL_IN_MIR_TOWER = 0x40;
constexpr uint8_t FLAG_SAW_MIRO_CLOSE_KEYDOOR = 0x08;

constexpr uint8_t FLAG_TALKED_WITH_ARTHUR_REVEALING_DUKE_ESCAPE = 0x40;         
constexpr uint8_t FLAG_ZAK_FLEW_OFF_MERCATOR_DUNGEON = 0x20;
constexpr uint8_t FLAG_SAW_PRINCESS_LARA_IN_MERCATOR_DUNGEON = 0x10;

constexpr uint8_t FLAG_DEX_KILLED = 0x80;

constexpr uint8_t FLAG_PRISONERS_OPENED_UNFINISHED_TUNNEL = 0x20;
constexpr uint8_t FLAG_OPENED_MERCATOR_TELEPORTER_TO_KNC = 0x02;

constexpr uint8_t FLAG_SAW_DUKE_MERCATOR_ON_RAFT_NEAR_DESTEL = 0x80;
constexpr uint8_t FLAG_SAW_DEXTER_DIE_IN_CRYPT = 0x40;
constexpr uint8_t FLAG_SAW_KAYLA_CUTSCENE_IN_GREENMAZE = 0x20;

constexpr uint8_t FLAG_FRIDAY_TALKED_AFTER_GETTING_LANTERN = 0x40;

constexpr uint8_t FLAG_SAW_ZAK_ON_BRIDGE = 0x80;
constexpr uint8_t FLAG_SAW_DUKE_ENTERING_LAKE_SHRINE = 0x01;
 
constexpr uint8_t FLAG_SAW_DUKE_TAUNTING_IN_SHELL_BREAST_ROOM = 0x80;
constexpr uint8_t FLAG_SECOND_RAFT_PLACED_IN_KNL = 0x02;

static void setup_story_flags(const RandomizerOptions& options, const World& world, std::vector<uint8_t>& out_flag_array)
{
    out_flag_array[0x00] = FLAG_NIGEL_AWOKEN_FROM_KNC_WATERFALL 
                         | FLAG_INTRO_FINISHED 
                         | FLAG_KEYDOOR_OPENED_IN_KNC 
                         | FLAG_FARA_KIDNAPPED;

    out_flag_array[0x02] = FLAG_TALKED_WITH_HIDDEN_VILLAGER_RYUMA 
                         | FLAG_SPIED_ON_GUMI_RITUAL 
                         | FLAG_BRIDGE_TO_GUMI_REPAIRED
                         | FLAG_MASSAN_VILLAGER_WARNED_BRIDGE_IS_BROKEN;
    
    out_flag_array[0x03] = FLAG_FIRST_ENCOUNTER_WITH_DUKE 
                         | FLAG_SAW_FRIDAY_CUTSCENE_AT_MADAME_YARD 
                         | FLAG_ARTHUR_GAVE_ACCESS_TO_CASTLE;

    out_flag_array[0x04] = FLAG_TALKED_WITH_MERCATOR_GUARDS_ASKING_GOLDS 
                         | FLAG_SAW_KAYLA_RAFT_CUTSCENE_AT_RYUMA 
                         | FLAG_ALLOWED_TO_GO_IN_THIEVES_HIDEOUT;

    out_flag_array[0x05] = FLAG_TALKED_WITH_MADAME_YARD_GIRL_HINTING_FORTUNE_TELLER 
                         | FLAG_SAW_ARTHUR_AT_CASTLE_BACKDOOR;

    // Various keydoors...
    out_flag_array[0x06] = 0x40 
                         | 0x20 
                         | 0x10 
                         | 0x08 
                         | 0x04;
    // Various keydoors...
    out_flag_array[0x07] = 0x80 
                         | 0x20 
                         | 0x10 
                         | 0x04;
    // Various keydoors...
    out_flag_array[0x08] = 0x80
                         | 0x40
                         | 0x20
                         | 0x10
                         | 0x08
                         | 0x04
                         | 0x02;
    // Various keydoors...
    out_flag_array[0x09] = 0x40 
                         | 0x20 
                         | 0x10 
                         | 0x08 
                         | 0x04;

    out_flag_array[0x12] = FLAG_ASSISTED_TO_MERCATOR_BANQUET 
                         | FLAG_TALKED_WITH_KITCHEN_WOMAN_BEFORE_BANQUET
                         | FLAG_SERVANT_BORUGHT_US_TO_BEDROOM
                         | FLAG_DUKE_LEFT_ROOM_AFTER_FIRST_ENCOUNTER;

    out_flag_array[0x13] = FLAG_COMPLETED_MERCATOR_JARS_SIDEQUEST_TWO_TIMES 
                         | FLAG_SAW_TRAPPED_KAYLA_CUTSCENE_IN_DUNGEON
                         | FLAG_THREW_IN_DUNGEON_BY_DUKE;

    out_flag_array[0x14] = FLAG_SAW_KAYLA_LITHOGRAPH_CUTSCENE
                         | FLAG_SLEPT_AT_MERCATOR_AFTER_BANQUET;

    out_flag_array[0x17] = FLAG_TOOK_PURPLE_JEWEL_IN_MIR_TOWER
                         | FLAG_SAW_MIRO_CLOSE_KEYDOOR;

    out_flag_array[0x20] = FLAG_TALKED_WITH_ARTHUR_REVEALING_DUKE_ESCAPE 
                         | FLAG_ZAK_FLEW_OFF_MERCATOR_DUNGEON 
                         | FLAG_SAW_PRINCESS_LARA_IN_MERCATOR_DUNGEON;
    
    out_flag_array[0x26] = FLAG_DEX_KILLED;
    
    out_flag_array[0x27] = FLAG_PRISONERS_OPENED_UNFINISHED_TUNNEL
                         | FLAG_OPENED_MERCATOR_TELEPORTER_TO_KNC;

    out_flag_array[0x28] = FLAG_SAW_DUKE_MERCATOR_ON_RAFT_NEAR_DESTEL 
                         | FLAG_SAW_DEXTER_DIE_IN_CRYPT 
                         | FLAG_SAW_KAYLA_CUTSCENE_IN_GREENMAZE;

    out_flag_array[0x29] = FLAG_FRIDAY_TALKED_AFTER_GETTING_LANTERN;

    out_flag_array[0x2A] = FLAG_SAW_ZAK_ON_BRIDGE
                         | FLAG_SAW_DUKE_ENTERING_LAKE_SHRINE;

    out_flag_array[0x2B] = FLAG_SAW_DUKE_TAUNTING_IN_SHELL_BREAST_ROOM
                         | FLAG_SECOND_RAFT_PLACED_IN_KNL;

    // If trees are considered visited at start, set all flags related to entering teleport trees
    if(options.all_trees_visited_at_start())
    {
        out_flag_array[0x23] = 0xFF;
        out_flag_array[0x25] |= 0x03;
    }

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
    // The ingame-tracker consists in putting in "grayed-out" key items in the inventory,
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

    // Remove node check on game boot
    disable_region_check(rom);
}
