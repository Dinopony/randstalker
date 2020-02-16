#include "World.h"

#include "Constants/ItemCodes.h"
#include "Constants/ItemSourceCodes.h"
#include "Constants/RegionCodes.h"
#include "Item.h"

#include "ItemChest.h"
#include "ItemOnGround.h"
#include "ItemInShop.h"
#include "ItemReward.h"

World::World(const RandomizerOptions& options) :
    spawnMapID(0x258), spawnX(0x1F), spawnZ(0x19)
{
    this->initItems();
    if (options.useArmorUpgrades())
        this->replaceArmorsByArmorUpgrades();

    this->initChests();
    this->initGroundItems();
    this->initShops();
    this->initNPCRewards();

    this->initRegions();
    this->initRegionPaths();

    if(options.shuffleTiborTrees())
        this->initTreeMaps();
}

World::~World()
{
	for (auto& [key, item] : items)
		delete item;
	for (auto& [key, itemSource] : itemSources)
		delete itemSource;
    for (auto& [key, region] : regions)
		delete region;
    for (ItemShop* shop : shops)
        delete shop;
}

void World::writeToROM(GameROM& rom)
{
	for (auto& [key, item] : items)
		item->writeToROM(rom);
	for (auto& [key, itemSource] : itemSources)
		itemSource->writeToROM(rom);
	for (const TreeMap& treeMap : treeMaps)
		treeMap.writeToROM(rom);

	rom.setWord(0x0027F4, spawnMapID);
	rom.setByte(0x0027FD, spawnX);
	rom.setByte(0x002805, spawnZ);
}

void World::initItems()
{
    items[ITEM_EKEEKE] =               new Item(ITEM_EKEEKE, "EkeEke", 20, true);
    items[ITEM_MAGIC_SWORD] =          new Item(ITEM_MAGIC_SWORD, "Magic Sword", 300, true);
    items[ITEM_ICE_SWORD] =            new Item(ITEM_ICE_SWORD, "Sword of Ice", 300, true);
    items[ITEM_THUNDER_SWORD] =        new Item(ITEM_THUNDER_SWORD, "Thunder Sword", 500, true);
    items[ITEM_GAIA_SWORD] =           new Item(ITEM_GAIA_SWORD, "Sword of Gaia", 300, true);
    items[ITEM_FIREPROOF_BOOTS] =      new Item(ITEM_FIREPROOF_BOOTS, "Fireproof", 150, true);
    items[ITEM_IRON_BOOTS] =           new Item(ITEM_IRON_BOOTS, "Iron Boots", 150, true);
    items[ITEM_HEALING_BOOTS] =        new Item(ITEM_HEALING_BOOTS, "Healing Boots", 300, true);
    items[ITEM_SPIKE_BOOTS] =          new Item(ITEM_SPIKE_BOOTS, "Snow Spikes", 400, true);
    items[ITEM_STEEL_BREAST] =         new Item(ITEM_STEEL_BREAST, "Steel Breast", 300, true);
    items[ITEM_CHROME_BREAST] =        new Item(ITEM_CHROME_BREAST, "Chrome Breast", 400, true);
    items[ITEM_SHELL_BREAST] =         new Item(ITEM_SHELL_BREAST, "Shell Breast", 500, true);
    items[ITEM_HYPER_BREAST] =         new Item(ITEM_HYPER_BREAST, "Hyper Breast", 750, true); 
    items[ITEM_MARS_STONE] =           new Item(ITEM_MARS_STONE, "Mars Stone", 150, true);
    items[ITEM_MOON_STONE] =           new Item(ITEM_MOON_STONE, "Moon Stone", 150, true);
    items[ITEM_SATURN_STONE] =         new Item(ITEM_SATURN_STONE, "Saturn Stone", 200, true);
    items[ITEM_VENUS_STONE] =          new Item(ITEM_VENUS_STONE, "Venus Stone", 300, true);
    items[ITEM_DETOX_GRASS] =          new Item(ITEM_DETOX_GRASS, "Detox Grass", 20, true);
    items[ITEM_GAIA_STATUE] =          new Item(ITEM_GAIA_STATUE, "Statue of Gaia", 100, true);
    items[ITEM_GOLDEN_STATUE] =        new Item(ITEM_GOLDEN_STATUE, "Golden Statue", 200);
    items[ITEM_MIND_REPAIR] =          new Item(ITEM_MIND_REPAIR, "Mind Repair", 20, true);
    items[ITEM_CASINO_TICKET] =        new Item(ITEM_CASINO_TICKET, "Casino Ticket", 50); // What to do with them?
    items[ITEM_AXE_MAGIC] =            new Item(ITEM_AXE_MAGIC, "Axe Magic", 400, true);
    items[ITEM_BLUE_RIBBON] =          new Item(ITEM_BLUE_RIBBON, "Blue Ribbon", 50, true); // What to do with it?
    items[ITEM_BUYER_CARD] =           new Item(ITEM_BUYER_CARD, "Buyer's Card", 100, true); // What to do with it?
    items[ITEM_LANTERN] =              new Item(ITEM_LANTERN, "Lantern", 150, true);
    items[ITEM_GARLIC] =               new Item(ITEM_GARLIC, "Garlic", 150, true);
    items[ITEM_ANTI_PARALYZE] =        new Item(ITEM_ANTI_PARALYZE, "Anti Paralyze", 20, true);
    items[ITEM_STATUE_JYPTA] =         new Item(ITEM_STATUE_JYPTA, "Statue of Jypta", 2000, true); // What to do with it?
    items[ITEM_SUN_STONE] =            new Item(ITEM_SUN_STONE, "Sun Stone", 400, true);
    items[ITEM_ARMLET] =               new Item(ITEM_ARMLET, "Armlet", 300, true);
    items[ITEM_EINSTEIN_WHISTLE] =     new Item(ITEM_EINSTEIN_WHISTLE, "Einstein Whistle", 300, true);
    items[ITEM_SPELL_BOOK] =           new Item(ITEM_SPELL_BOOK, "Spell Book", 50, true); // What to do with it?
    items[ITEM_LITHOGRAPH] =           new Item(ITEM_LITHOGRAPH, "Lithograph", 100, true); // What to do with it?
    items[ITEM_RED_JEWEL] =            new Item(ITEM_RED_JEWEL, "Red Jewel", 100, true); // What to do with it?
    items[ITEM_PAWN_TICKET] =          new Item(ITEM_PAWN_TICKET, "Pawn Ticket", 100, true);
    items[ITEM_PURPLE_JEWEL] =         new Item(ITEM_PURPLE_JEWEL, "Purple Jewel", 100); // What to do with it?
    items[ITEM_GOLA_EYE] =             new Item(ITEM_GOLA_EYE, "Gola's Eye", 500, true);
    items[ITEM_DEATH_STATUE] =         new Item(ITEM_DEATH_STATUE, "Death Statue", 150, true);
    items[ITEM_DAHL] =                 new Item(ITEM_DAHL, "Dahl", 100);
    items[ITEM_RESTORATION] =          new Item(ITEM_RESTORATION, "Restoration", 40, true);
    items[ITEM_LOGS] =                 new Item(ITEM_LOGS, "Logs", 200, true);
    items[ITEM_ORACLE_STONE] =         new Item(ITEM_ORACLE_STONE, "Oracle Stone", 100, true); // What to do with it?
    items[ITEM_IDOL_STONE] =           new Item(ITEM_IDOL_STONE, "Idol Stone", 200, true);
    items[ITEM_KEY] =                  new Item(ITEM_KEY, "Key", 150, true);
    items[ITEM_SAFETY_PASS] =          new Item(ITEM_SAFETY_PASS, "Safety Pass", 300, true);
    items[ITEM_BELL] =                 new Item(ITEM_BELL, "Bell", 200, true);
    items[ITEM_SHORT_CAKE] =           new Item(ITEM_SHORT_CAKE, "Short Cake", 100, true); // What to do with it?
    items[ITEM_GOLA_NAIL] =            new Item(ITEM_GOLA_NAIL, "Gola's Nail", 1000, true);
    items[ITEM_GOLA_HORN] =            new Item(ITEM_GOLA_HORN, "Gola's Horn", 1000, true);
    items[ITEM_GOLA_FANG] =            new Item(ITEM_GOLA_FANG, "Gola's Fang", 1000, true);
    items[ITEM_LIFESTOCK] =            new Item(ITEM_LIFESTOCK, "Life Stock", 200);
    items[ITEM_NONE] =                 new Item(ITEM_NONE, "No Item", 0);

    for (uint8_t i = 0; i < GOLD_SOURCES_COUNT; ++i)
        items[ITEM_GOLDS_START + i] = new Item(ITEM_GOLDS_START + i, "", 0);

    // The following items are placeholder items, thus they are missing from this list:
    //      - Awakening Book (0x11)
    //      - Detox Book (0x21)
    //      - AntiCurse (0x22)
    //      - Record Book (0x23)
    //      - Hotel Register (0x25)
    //      - Island Map (0x26)
    //      - No52 (0x34)
}

void World::replaceArmorsByArmorUpgrades()
{
    std::vector<Item*> armors = { items[ITEM_STEEL_BREAST], items[ITEM_CHROME_BREAST], items[ITEM_SHELL_BREAST], items[ITEM_HYPER_BREAST] };
    for (uint8_t i = 0; i < 4; ++i)
    {
        armors[i]->setName(std::string("Armor upgrade ").append(1, '1'+i));
        armors[i]->setGoldWorth(250);
    }
}

void World::initChests()
{
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_RIGHT_EKEEKE] =              new ItemChest(0x00, ITEM_EKEEKE,            "Swamp Shrine (0F): ekeeke chest in room to the right, close to door");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_CARPET_KEY] =                new ItemChest(0x01, ITEM_KEY,               "Swamp Shrine (0F): key chest in carpet room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_LIFESTOCK] =                 new ItemChest(0x02, ITEM_LIFESTOCK,         "Swamp Shrine (0F): lifestock chest in room to the left, accessible by falling from upstairs");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FALLING_EKEEKE] =            new ItemChest(0x03, ITEM_EKEEKE,            "Swamp Shrine (0F): ekeeke chest falling from the ceiling when beating the orc");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FRONT_EKEEKE] =              new ItemChest(0x04, ITEM_EKEEKE,            "Swamp Shrine (0F): ekeeke chest in room connected to second entrance (without idol stone)");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_LOWER_BRIDGES_KEY] =         new ItemChest(0x05, ITEM_KEY,               "Swamp Shrine (1F): lower key chest in wooden bridges room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_UPPER_BRIDGES_KEY] =         new ItemChest(0x06, ITEM_KEY,               "Swamp Shrine (2F): upper key chest in wooden bridges room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_2F_SPIKE_KEY] =                 new ItemChest(0x07, ITEM_KEY,               "Swamp Shrine (2F): key chest in spike room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_3F_REWARD] =                    new ItemChest(0x08, ITEM_LIFESTOCK,         "Swamp Shrine (3F): lifestock chest in Fara's room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_LIFESTOCK] =    new ItemChest(0x09, ITEM_LIFESTOCK,         "Mercator Dungeon (-1F): lifestock chest after key door");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_KEY] =                      new ItemChest(0x0A, ITEM_KEY,               "Mercator Dungeon (-1F): key chest in Moralis's cell");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_LEFT] = new ItemChest(0x0B, ITEM_EKEEKE,        "Mercator Dungeon (-1F): left ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_RIGHT] = new ItemChest(0x0C, ITEM_EKEEKE,       "Mercator Dungeon (-1F): right ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASTLE_KITCHEN] =                   new ItemChest(0x0D, ITEM_LIFESTOCK,         "Mercator: castle kitchen chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_SICK_MERCHANT] =                    new ItemChest(0x0E, ITEM_BUYER_CARD,        "Mercator: chest replacing sick merchant in secondary shop backroom");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_LEFT] =   new ItemChest(0x0F, ITEM_EKEEKE,            "Mercator Dungeon (1F): left ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_RIGHT] =  new ItemChest(0x10, ITEM_EKEEKE,            "Mercator Dungeon (1F): right ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_ARTHUR_KEY] =                       new ItemChest(0x11, ITEM_KEY,               "Mercator: Arthur key chest in castle tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_LIFESTOCK] =          new ItemChest(0x12, ITEM_LIFESTOCK,         "Mercator Dungeon (4F): chest on top of tower");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_LIFESTOCK] =                       new ItemChest(0x13, ITEM_LIFESTOCK,         "King Nole's Palace: entrance lifestock chest");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_EKEEKE] =                          new ItemChest(0x14, ITEM_EKEEKE,            "King Nole's Palace: ekeeke chest in topmost pit room");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_DAHL] =                            new ItemChest(0x15, ITEM_DAHL,              "King Nole's Palace: dahl chest in floating button room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_LIFESTOCK] =                   new ItemChest(0x16, ITEM_LIFESTOCK,         "King Nole's Cave: first lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_FARA_REWARD] =                        new ItemChest(0x17, ITEM_RED_JEWEL,         "Massan: chest in elder house after freeing Fara");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_GOLD] =                        new ItemChest(0x18, ITEM_5_GOLDS,           "King Nole's Cave: first gold chest in third room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_GOLD] =                       new ItemChest(0x19, ITEM_5_GOLDS,           "King Nole's Cave: second gold chest in third room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_LIFESTOCK] =            new ItemChest(0x1A, ITEM_LIFESTOCK,         "Greenmaze: chest on path to lumberjack");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_WHISTLE] =              new ItemChest(0x1B, ITEM_EINSTEIN_WHISTLE,  "Greenmaze: chest replacing lumberjack");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_THIRD_GOLD] =                        new ItemChest(0x1C, ITEM_5_GOLDS,           "King Nole's Cave: gold chest in isolated room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_LIFESTOCK] =                  new ItemChest(0x1D, ITEM_LIFESTOCK,         "King Nole's Cave: lifestock chest in crate room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_BOULDER_CHASE_LIFESTOCK] =           new ItemChest(0x1F, ITEM_LIFESTOCK,         "King Nole's Cave: boulder chase corridor chest");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_ENTRANCE_LIFESTOCK] =       new ItemChest(0x21, ITEM_LIFESTOCK,         "Waterfall Shrine: lifestock chest under entrance (accessible after talking with Prospero)");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_END_LIFESTOCK] =            new ItemChest(0x22, ITEM_LIFESTOCK,         "Waterfall Shrine: lifestock chest near Prospero");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_GOLDS] =                    new ItemChest(0x23, ITEM_5_GOLDS,           "Waterfall Shrine: chest in button room");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_KEY] =                      new ItemChest(0x24, ITEM_KEY,               "Waterfall Shrine: upstairs key chest");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_ENTRANCE_EKEEKE] =           new ItemChest(0x26, ITEM_EKEEKE,            "Thieves Hideout: chest in entrance room when water is removed");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_LEFT] =             new ItemChest(0x29, ITEM_KEY,               "Thieves Hideout: right chest in room accessible by falling from miniboss room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_RIGHT] =            new ItemChest(0x2A, ITEM_EKEEKE,            "Thieves Hideout: left chest in room accessible by falling from miniboss room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_LEFT] =         new ItemChest(0x2B, ITEM_KEY,               "Thieves Hideout: left chest in Pockets cell");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_RIGHT] =        new ItemChest(0x2C, ITEM_LIFESTOCK,         "Thieves Hideout: right chest in Pockets cell");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_RIGHT] =         new ItemChest(0x2D, ITEM_EKEEKE,            "Thieves Hideout: right chest in room after quick climb trial");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_LEFT] =          new ItemChest(0x2E, ITEM_GAIA_STATUE,       "Thieves Hideout: left chest in room after quick climb trial");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_FIRST_PLATFORM_ROOM] =       new ItemChest(0x2F, ITEM_EKEEKE,            "Thieves Hideout: chest in first platform room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_SECOND_PLATFORM_ROOM] =      new ItemChest(0x30, ITEM_EKEEKE,            "Thieves Hideout: chest in second platform room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MOVING_BALLS_RIDDLE] =       new ItemChest(0x31, ITEM_LIFESTOCK,         "Thieves Hideout: reward chest after moving balls room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_EKEEKE_ROLLING_BOULDER] =    new ItemChest(0x32, ITEM_EKEEKE,            "Thieves Hideout: chest near rolling boulder");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_LEFT] =          new ItemChest(0x34, ITEM_EKEEKE,            "Thieves Hideout: left chest in double ekeeke chest room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_RIGHT] =         new ItemChest(0x35, ITEM_EKEEKE,            "Thieves Hideout: right chest in double ekeeke chest room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_LEFT] =          new ItemChest(0x36, ITEM_20_GOLDS,          "Thieves Hideout: left chest in room before boss");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_RIGHT] =         new ItemChest(0x37, ITEM_GOLDEN_STATUE,     "Thieves Hideout: right chest in room before boss");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LITHOGRAPH] =         new ItemChest(0x38, ITEM_LITHOGRAPH,        "Thieves Hideout: lithograph chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_5G] =                 new ItemChest(0x39, ITEM_5_GOLDS,           "Thieves Hideout: 5 golds chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_50G] =                new ItemChest(0x3A, ITEM_50_GOLDS,          "Thieves Hideout: 50 golds chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LIFESTOCK] =          new ItemChest(0x3B, ITEM_LIFESTOCK,         "Thieves Hideout: lifestock chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_EKEEKE] =             new ItemChest(0x3C, ITEM_EKEEKE,            "Thieves Hideout: ekeeke chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_LEFT] =       new ItemChest(0x42, ITEM_LIFESTOCK,         "Verla Mines: right chest in \"crate on spike\" room near entrance");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_RIGHT] =      new ItemChest(0x43, ITEM_EKEEKE,            "Verla Mines: left chest in \"crate on spike\" room near entrance");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_JAR_STAIRCASE_ROOM] =            new ItemChest(0x44, ITEM_LIFESTOCK,         "Verla Mines: chest on isolated cliff in \"jar staircase\" room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_DEX_KEY] =                       new ItemChest(0x45, ITEM_KEY,               "Verla Mines: Dex reward chest");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_SLASHER_KEY] =                   new ItemChest(0x46, ITEM_KEY,               "Verla Mines: Slasher reward chest");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_LEFT] =                     new ItemChest(0x47, ITEM_GOLDEN_STATUE,     "Verla Mines: left chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_MIDDLE] =                   new ItemChest(0x48, ITEM_EKEEKE,            "Verla Mines: middle chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_RIGHT] =                    new ItemChest(0x49, ITEM_EKEEKE,            "Verla Mines: right chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_RIGHT] =                new ItemChest(0x4A, ITEM_EKEEKE,            "Verla Mines: right chest in button room near elevator shaft leading to Marley");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_LEFT] =                 new ItemChest(0x4B, ITEM_EKEEKE,            "Verla Mines: left chest in button room near elevator shaft leading to Marley");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_LAVA_WALKING] =                  new ItemChest(0x4C, ITEM_LIFESTOCK,         "Verla Mines: chest in hidden room accessible by lava-walking");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_CRATES_KEY] =                    new ItemChest(0x4D, ITEM_KEY,               "Destel Well (0F): \"crates and holes\" room key chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_STAIRS_EKEEKE] =                 new ItemChest(0x4E, ITEM_EKEEKE,            "Destel Well (1F): ekeeke chest on small stairs");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_NARROW_GROUND_LIFESTOCK] =       new ItemChest(0x4F, ITEM_LIFESTOCK,         "Destel Well (1F): lifestock chest on narrow ground");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_SPIKE_HALLWAY] =                 new ItemChest(0x50, ITEM_LIFESTOCK,         "Destel Well (1F): lifestock chest in spike room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_SIDE_LIFESTOCK] =             new ItemChest(0x51, ITEM_LIFESTOCK,         "Destel Well (2F): lifestock chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_DAHL] =                       new ItemChest(0x52, ITEM_DAHL,              "Destel Well (2F): dahl chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_RIGHT] =            new ItemChest(0x53, ITEM_LIFESTOCK,         "Destel Well (2F): right chest in Pockets room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_LEFT] =             new ItemChest(0x54, ITEM_GAIA_STATUE,       "Destel Well (2F): left chest in Pockets room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_1] =                   new ItemChest(0x55, ITEM_KEY,               "Destel Well (3F): key chest in first trigger room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_2] =                   new ItemChest(0x56, ITEM_KEY,               "Destel Well (3F): key chest in giants room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_3] =                   new ItemChest(0x57, ITEM_KEY,               "Destel Well (3F): key chest in second trigger room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_TOP] =                new ItemChest(0x58, ITEM_MIND_REPAIR,       "Destel Well (4F): top chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_LEFT] =               new ItemChest(0x59, ITEM_DAHL,              "Destel Well (4F): left chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_DOWN] =               new ItemChest(0x5A, ITEM_EKEEKE,            "Destel Well (4F): down chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_RIGHT] =              new ItemChest(0x5B, ITEM_EKEEKE,            "Destel Well (4F): right chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_KEY] =          new ItemChest(0x5C, ITEM_KEY,               "Lake Shrine (-1F): green golem spinner key chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLDEN_STATUE] =              new ItemChest(0x5D, ITEM_GOLDEN_STATUE,     "Lake Shrine (-1F): golden statue chest in corridor");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_LIFESTOCK] =    new ItemChest(0x5E, ITEM_LIFESTOCK,         "Lake Shrine (-1F): green golem spinner lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLEM_HOPPING_LIFESTOCK] =    new ItemChest(0x5F, ITEM_LIFESTOCK,         "Lake Shrine (-1F): golem hopping lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_LIFESTOCK_FALLING_FROM_UNICORNS] = new ItemChest(0x60, ITEM_LIFESTOCK,    "Lake Shrine (-2F): middle life stock");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_LIFESTOCK] =      new ItemChest(0x61, ITEM_LIFESTOCK,         "Lake Shrine (-2F): \"throne room\" lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_KEY] =            new ItemChest(0x62, ITEM_KEY,               "Lake Shrine (-2F): \"throne room\" key chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_WHITE_GOLEMS_CEILING] =       new ItemChest(0x63, ITEM_LIFESTOCK,         "Lake Shrine (-3F): white golems room");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_KEY_NEAR_SWORD] =             new ItemChest(0x64, ITEM_KEY,               "Lake Shrine (-3F): key chest near sword of ice");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_SNAKE_CAGING_RIDDLE] =        new ItemChest(0x65, ITEM_LIFESTOCK,         "Lake Shrine (-3F): chest in snake caging room");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_LIFESTOCK_FALLING_FROM_PLATFORMS] = new ItemChest(0x66, ITEM_LIFESTOCK,   "Lake Shrine (-3F): lifestock chest on central block, obtained by falling from above");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_HALLWAY_TO_DUKE] =            new ItemChest(0x67, ITEM_DAHL,              "Lake Shrine (-3F): chest before reaching the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_LEFT] =                new ItemChest(0x68, ITEM_EKEEKE,            "Lake Shrine (-3F): reward chest (left) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_MIDDLE] =              new ItemChest(0x69, ITEM_LIFESTOCK,         "Lake Shrine (-3F): reward chest (middle) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_RIGHT] =               new ItemChest(0x6A, ITEM_EKEEKE,            "Lake Shrine (-3F): reward chest (right) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_GOLDEN_SPINNER_KEY] =         new ItemChest(0x6B, ITEM_KEY,               "Lake Shrine (-3F): key chest near golden golem spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EXTERIOR_KEY] =              new ItemChest(0x6C, ITEM_KEY,               "King Nole's Labyrinth (0F): key chest in \"outside room\"");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EKEEKE_AFTER_KEYDOOR] =      new ItemChest(0x6D, ITEM_EKEEKE,            "King Nole's Labyrinth (0F): ekeeke chest in room after key door");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_LIFESTOCK_AFTER_KEYDOOR] =   new ItemChest(0x6E, ITEM_LIFESTOCK,         "King Nole's Labyrinth (0F): lifestock chest in room after key door");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SMALL_MAZE_LIFESTOCK] =      new ItemChest(0x6F, ITEM_LIFESTOCK,         "King Nole's Labyrinth (-1F): lifestock chest in \"small maze\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SPIKE_BALLS_GAIA_STATUE] =   new ItemChest(0x70, ITEM_GAIA_STATUE,       "King Nole's Labyrinth (0F): chest in spike balls room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_1] =          new ItemChest(0x71, ITEM_EKEEKE,            "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (left side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_2] =          new ItemChest(0x72, ITEM_EKEEKE,            "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (right side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_3] =          new ItemChest(0x73, ITEM_RESTORATION,       "King Nole's Labyrinth (-1F): restoration chest in triple chest dark room (left side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_BIG_MAZE_LIFESTOCK] =        new ItemChest(0x74, ITEM_LIFESTOCK,         "King Nole's Labyrinth (-1F): lifestock chest in \"big maze\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN_ROOM_EKEEKE] =       new ItemChest(0x75, ITEM_EKEEKE,            "King Nole's Labyrinth (-1F): ekeeke chest in lantern room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN] =                   new ItemChest(0x76, ITEM_LANTERN,           "King Nole's Labyrinth (-1F): lantern chest");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_ICE_SHORTCUT_KEY] =          new ItemChest(0x77, ITEM_KEY,               "King Nole's Labyrinth (-1F): key chest in ice shortcut room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_SAVE_ROOM] =                 new ItemChest(0x78, ITEM_EKEEKE,            "King Nole's Labyrinth (-2F): ekeeke chest in skeleton priest room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_BUTTON_CRATES_KEY] =         new ItemChest(0x79, ITEM_KEY,               "King Nole's Labyrinth (-1F): key chest in \"button and crates\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_EKEEKE] =          new ItemChest(0x7A, ITEM_EKEEKE,            "King Nole's Labyrinth (-3F): ekeeke chest before Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_DAHL] =            new ItemChest(0x7B, ITEM_DAHL,              "King Nole's Labyrinth (-3F): dahl chest before Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_REWARD] =          new ItemChest(0x7C, ITEM_GOLA_NAIL,         "King Nole's Labyrinth (-3F): reward for beating Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FOUR_BUTTONS_RIDDLE] =       new ItemChest(0x7D, ITEM_LIFESTOCK,         "King Nole's Labyrinth (-2F): lifestock chest in four buttons room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_1] =          new ItemChest(0x7E, ITEM_EKEEKE,            "King Nole's Labyrinth (-3F): first ekeeke chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_2] =          new ItemChest(0x7F, ITEM_EKEEKE,            "King Nole's Labyrinth (-3F): second ekeeke chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_GAIA_STATUE] =       new ItemChest(0x80, ITEM_GAIA_STATUE,       "King Nole's Labyrinth (-3F): statue of gaia chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_REWARD] =            new ItemChest(0x81, ITEM_GOLA_FANG,         "King Nole's Labyrinth (-3F): reward for beating Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_KEY] =               new ItemChest(0x82, ITEM_KEY,               "King Nole's Labyrinth (-3F): key chest in Hyper Breast room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_GARLIC] =               new ItemChest(0x83, ITEM_GARLIC,            "King Nole's Labyrinth (-3F): chest before Miro");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_REWARD] =               new ItemChest(0x84, ITEM_GOLA_HORN,         "King Nole's Labyrinth (-3F): reward for beating Miro");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_DEVIL_HANDS] =               new ItemChest(0x85, ITEM_LIFESTOCK,         "King Nole's Labyrinth (-3F): chest in hands room");
    itemSources[ItemSourceCode::CHEST_ROUTE_GUMI_RYUMA_LIFESTOCK] =                new ItemChest(0x86, ITEM_LIFESTOCK,         "Route between Gumi and Ryuma: chest on the way to Swordsman Kado");
    itemSources[ItemSourceCode::CHEST_ROUTE_MASSAN_GUMI_PROMONTORY] =              new ItemChest(0x87, ITEM_LIFESTOCK,         "Route between Massan and Gumi: chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_GOLDEN_STATUE] =            new ItemChest(0x88, ITEM_GOLDEN_STATUE,     "Route between Mercator and Verla: golden statue chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_RESTORATION] =              new ItemChest(0x89, ITEM_RESTORATION,       "Route between Mercator and Verla: restoration chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TWINKLE_LIFESTOCK] =        new ItemChest(0x8A, ITEM_LIFESTOCK,         "Route between Mercator and Verla: chest near Friday's village");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_ANGLE_PROMONTORY] =             new ItemChest(0x8B, ITEM_LIFESTOCK,         "Verla Shore: chest on angle promontory after Verla tunnel");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_CLIFF_CHEST] =                  new ItemChest(0x8C, ITEM_LIFESTOCK,         "Verla Shore: chest on highest promontory after Verla tunnel (accessible through Verla mines)");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_HIDDEN_BUTTON_LIFESTOCK] =  new ItemChest(0x8D, ITEM_LIFESTOCK,         "Route to Mir Tower: chest on promontory accessed by pressing hidden switch");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_DAHL] =                new ItemChest(0x8E, ITEM_DAHL,              "Route to Mir Tower: dahl chest behind sacred tree");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_BEHIND_CABIN] =                 new ItemChest(0x8F, ITEM_LIFESTOCK,         "Verla Shore: chest behind cabin");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_BUSHES_DAHL] =            new ItemChest(0x90, ITEM_DAHL,              "Route to Destel: chest in map right after Verla mines exit");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_ELEVATOR] =               new ItemChest(0x91, ITEM_LIFESTOCK,         "Route to Destel: chest in \"elevator\" map");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_LIFESTOCK] =           new ItemChest(0x92, ITEM_LIFESTOCK,         "Route to Mir Tower: lifestock chest behind sacred tree");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_HIDDEN_LIFESTOCK] =       new ItemChest(0x93, ITEM_LIFESTOCK,         "Route to Destel: hidden chest in map right before Destel");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_DAHL_NEAR_TREE] =           new ItemChest(0x94, ITEM_DAHL,              "Mountainous Area: chest near teleport tree");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LIFESTOCK_BEFORE_BRIDGE] =  new ItemChest(0x95, ITEM_LIFESTOCK,         "Mountainous Area: chest on right side of the map right before the bridge");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GOLDEN_STATUE] =     new ItemChest(0x96, ITEM_GOLDEN_STATUE,     "Mountainous Area: hidden chest in narrow path");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GAIA_STATUE] =       new ItemChest(0x97, ITEM_GAIA_STATUE,       "Mountainous Area: hidden Statue of Gaia chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_CLIFF_LIFESTOCK] =   new ItemChest(0x98, ITEM_LIFESTOCK,         "Mountainous Area: isolated life stock in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_LEFT] =         new ItemChest(0x99, ITEM_EKEEKE,            "Mountainous Area: left chest on wall in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_RIGHT] =        new ItemChest(0x9A, ITEM_GOLDEN_STATUE,     "Mountainous Area: right chest on wall in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_RESTORATION_NEAR_ZAK] =     new ItemChest(0x9B, ITEM_RESTORATION,       "Mountainous Area: restoration chest in map before Zak arena");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_ZAK] =          new ItemChest(0x9C, ITEM_EKEEKE,            "Mountainous Area: ekeeke chest in map before Zak arena");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_CORNER_LIFESTOCK] =       new ItemChest(0x9D, ITEM_LIFESTOCK,         "Route after Destel: chest on cliff angle");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_HIDDEN_LIFESTOCK] =       new ItemChest(0x9E, ITEM_LIFESTOCK,         "Route after Destel: lifestock chest in map after seeing Duke raft");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_DAHL] =                   new ItemChest(0x9F, ITEM_DAHL,              "Route after Destel: dahl chest in map after seeing Duke raft");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BELOW_ROCKY_ARCH] =         new ItemChest(0xA0, ITEM_LIFESTOCK,         "Mountainous Area: chest hidden under rocky arch");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_EASY_LIFESTOCK] =          new ItemChest(0xA1, ITEM_LIFESTOCK,         "Route to Lake Shrine: \"easy\" chest on crossroads with mountainous area");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LAKE_SHRINE_SHORTCUT] =     new ItemChest(0xA2, ITEM_LIFESTOCK,         "Route to Lake Shrine: \"hard\" chest on crossroads with mountainous area");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_BRIDGE] =       new ItemChest(0xA3, ITEM_EKEEKE,            "Mountainous Area: chest in map in front of the statue under the bridge");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_RIGHT] =           new ItemChest(0xA4, ITEM_GAIA_STATUE,       "Route to Lake Shrine: right chest in volcano");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_LEFT] =            new ItemChest(0xA5, ITEM_GAIA_STATUE,       "Route to Lake Shrine: left chest in volcano");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN] =              new ItemChest(0xA6, ITEM_LIFESTOCK,         "Mountainous Area Cave: chest in small hidden room");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_VISIBLE] =             new ItemChest(0xA7, ITEM_DAHL,              "Mountainous Area Cave: chest in small visible room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_PROMONTORY_GOLDS] =                new ItemChest(0xA9, ITEM_20_GOLDS,          "Greenmaze: chest on promontory appearing after pressing a button in other section");
    itemSources[ItemSourceCode::CHEST_MASSAN_SHORTCUT_DAHL] =                      new ItemChest(0xAA, ITEM_DAHL,              "Greenmaze: chest between Sunstone and Massan shortcut");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_MAGES_LIFESTOCK] =                 new ItemChest(0xAB, ITEM_LIFESTOCK,         "Greenmaze: chest in mages room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_LEFT] =                 new ItemChest(0xAC, ITEM_LIFESTOCK,         "Greenmaze: left chest in elbow cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_RIGHT] =                new ItemChest(0xAD, ITEM_DAHL,              "Greenmaze: right chest in elbow cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_DAHL] =             new ItemChest(0xAE, ITEM_DAHL,              "Greenmaze: chest in waterfall cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_LIFESTOCK] =        new ItemChest(0xAF, ITEM_LIFESTOCK,         "Greenmaze: left chest in hidden room behind waterfall ");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_GOLDS] =            new ItemChest(0xB0, ITEM_20_GOLDS,          "Greenmaze: right chest in hidden room behind waterfall ");
    itemSources[ItemSourceCode::CHEST_MASSAN_DOG_STATUE] =                         new ItemChest(0xB1, ITEM_LIFESTOCK,         "Massan: chest triggered by dog statue");
    itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_1] =                           new ItemChest(0xB2, ITEM_EKEEKE,            "Massan: chest in house nearest to elder house");
    itemSources[ItemSourceCode::CHEST_MASSAN_HOUSE_LIFESTOCK] =                    new ItemChest(0xB3, ITEM_LIFESTOCK,         "Massan: lifestock chest in house");
    itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_2] =                           new ItemChest(0xB4, ITEM_EKEEKE,            "Massan: chest in house farthest from elder house");
    itemSources[ItemSourceCode::CHEST_GUMI_LIFESTOCK] =                            new ItemChest(0xB5, ITEM_LIFESTOCK,         "Gumi: chest on top of bed in house");
    itemSources[ItemSourceCode::CHEST_GUMI_FARA_REWARD] =                          new ItemChest(0xB6, ITEM_LIFESTOCK,         "Gumi: chest in elder house after saving Fara");
    itemSources[ItemSourceCode::CHEST_RYUMA_MAYOR_LIFESTOCK] =                     new ItemChest(0xB7, ITEM_LIFESTOCK,         "Ryuma: chest in mayor's house");
    itemSources[ItemSourceCode::CHEST_RYUMA_LIGHTHOUSE_LIFESTOCK] =                new ItemChest(0xB8, ITEM_LIFESTOCK,         "Ryuma: chest in repaired lighthouse");
    itemSources[ItemSourceCode::CHEST_CRYPT_MAIN_LOBBY] =                          new ItemChest(0xB9, ITEM_LIFESTOCK,         "Crypt: chest in main room");
    itemSources[ItemSourceCode::CHEST_CRYPT_ARMLET] =                              new ItemChest(0xBA, ITEM_ARMLET,            "Crypt: reward chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO] =                           new ItemChest(0xBF, ITEM_DAHL,              "Mercator: hidden casino chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_GREENPEA] =                         new ItemChest(0xC0, ITEM_LIFESTOCK,         "Mercator: chest in Greenpea's house");
    itemSources[ItemSourceCode::CHEST_MERCATOR_GRANDMA_POT_SHELVING] =             new ItemChest(0xC1, ITEM_LIFESTOCK,         "Mercator: chest in grandma's house (pot shelving trial)");
    itemSources[ItemSourceCode::CHEST_VERLA_WELL] =                                new ItemChest(0xC2, ITEM_THUNDER_SWORD,     "Verla: chest in well after beating Marley");
    itemSources[ItemSourceCode::CHEST_DESTEL_INN_COUNTER] =                        new ItemChest(0xC4, ITEM_LIFESTOCK,         "Destel: chest in shop requiring to wait for the shopkeeper to move");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_GARLIC] =                          new ItemChest(0xC5, ITEM_GARLIC,            "Mir Tower: garlic chest");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_AFTER_MIMICS] =             new ItemChest(0xC6, ITEM_EKEEKE,            "Mir Tower: chest after mimic room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_1] =                         new ItemChest(0xC7, ITEM_NONE,              "Mir Tower: mimic room empty chest 1");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_2] =                         new ItemChest(0xC8, ITEM_NONE,              "Mir Tower: mimic room empty chest 2");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_3] =                         new ItemChest(0xC9, ITEM_NONE,              "Mir Tower: mimic room empty chest 3");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_4] =                         new ItemChest(0xCA, ITEM_NONE,              "Mir Tower: mimic room empty chest 4");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_MUSHROOM_PIT_ROOM] =               new ItemChest(0xCB, ITEM_EKEEKE,            "Mir Tower: chest in mushroom pit room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_GAIA_STATUE] =                     new ItemChest(0xCC, ITEM_GAIA_STATUE,       "Mir Tower: chest in room next to mummy switch room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_LIBRARY_LIFESTOCK] =               new ItemChest(0xCD, ITEM_LIFESTOCK,         "Mir Tower: chest in library accessible from teleporter maze");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_HIDDEN_EKEEKE] =                   new ItemChest(0xCE, ITEM_EKEEKE,            "Mir Tower: hidden chest in room before library");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_FALLING_SPIKEBALLS] =              new ItemChest(0xCF, ITEM_LIFESTOCK,         "Mir Tower: chest in falling spikeballs room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_TIMED_KEY] =                       new ItemChest(0xD0, ITEM_KEY,               "Mir Tower: chest in timed challenge room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_1] =             new ItemChest(0xD1, ITEM_EKEEKE,            "Mir Tower: chest in room where Miro closes the door");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_2] =             new ItemChest(0xD2, ITEM_EKEEKE,            "Mir Tower: chest after room where Miro closes the door");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_PURPLE_JEWEL] =             new ItemChest(0xD3, ITEM_PURPLE_JEWEL,      "Mir Tower: reward chest");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_RIGHT_EKEEKE] =             new ItemChest(0xD4, ITEM_EKEEKE,            "Mir Tower: right chest in reward room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LEFT_EKEEKE] =              new ItemChest(0xD5, ITEM_EKEEKE,            "Mir Tower: left chest in reward room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LIFESTOCK] =                new ItemChest(0xD6, ITEM_LIFESTOCK,         "Mir Tower: chest behind wall accessible after beating Mir");
    itemSources[ItemSourceCode::CHEST_HELGA] =                                     new ItemChest(0xD7, ITEM_LIFESTOCK,         "Witch Helga's Hut: lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_LIFESTOCK] =                     new ItemChest(0xD8, ITEM_LIFESTOCK,         "Massan Cave: lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_DAHL] =                          new ItemChest(0xD9, ITEM_DAHL,              "Massan Cave: dahl chest");
    itemSources[ItemSourceCode::CHEST_TIBOR_LIFESTOCK] =                           new ItemChest(0xDA, ITEM_LIFESTOCK,         "Tibor: reward chest after boss");
    itemSources[ItemSourceCode::CHEST_TIBOR_SPIKEBALLS_ROOM] =                     new ItemChest(0xDB, ITEM_20_GOLDS,          "Tibor: chest in spike balls room");
    itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_LEFT] =                           new ItemChest(0xDC, ITEM_GAIA_STATUE,       "Tibor: left chest on 2 chest group");
    itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_RIGHT] =                          new ItemChest(0xDD, ITEM_20_GOLDS,          "Tibor: right chest on 2 chest group");

    // The following chests are absent from the game on release or modded out of the game for the rando, and their IDs are therefore free:
    // 0x0E, 0x1E, 0x20, 0x25, 0x27, 0x28, 0x33, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0xA8, 0xBB, 0xBC, 0xBD, 0xBE, 0xC3
}

void World::initGroundItems()
{
    itemSources[ItemSourceCode::GROUND_IDOL_STONE] = new ItemOnGround(0x021167, "Gumi: Idol Stone on ground");
    itemSources[ItemSourceCode::GROUND_SUN_STONE] = new ItemOnGround(0x020C21, "Greenmaze: Sun Stone on ground");
    itemSources[ItemSourceCode::GROUND_CHROME_BREAST] = new ItemOnGround(0x01DDB7, "Verla Mines: Chrome Breast on ground");
    itemSources[ItemSourceCode::GROUND_SHELL_BREAST] = new ItemOnGround(0x01EC99, "Lake Shrine (-3F): Shell Breast on ground");
    itemSources[ItemSourceCode::GROUND_HYPER_BREAST] = new ItemOnGround(0x01F9BD, "King Nole's Labyrinth (-3F): Hyper Breast on ground");
    itemSources[ItemSourceCode::GROUND_HEALING_BOOTS] = new ItemOnGround(0x01E247, "Destel Well: Healing Boots on ground");
    itemSources[ItemSourceCode::GROUND_IRON_BOOTS] = new ItemOnGround(0x01F36F, "King Nole's Labyrinth (-1F): Iron Boots on ground");
    itemSources[ItemSourceCode::GROUND_FIREPROOF] = new ItemOnGround(0x022C23, "Massan Cave: Fireproof Boots on ground");
    itemSources[ItemSourceCode::GROUND_SPIKE_BOOTS] = new ItemOnGround(0x01FAC1, "King Nole's Labyrinth (-3F): Snow Spikes on ground");
    itemSources[ItemSourceCode::GROUND_GAIA_SWORD] = new ItemOnGround(0x01F183, "King Nole's Labyrinth (-2F): Sword of Gaia on ground");
    itemSources[ItemSourceCode::GROUND_MARS_STONE] = new ItemOnGround(0x020419, "Route after Destel: Mars Stone on ground");
    itemSources[ItemSourceCode::GROUND_MOON_STONE] = new ItemOnGround(0x020AED, "Mountainous Area cave: Moon Stone on ground");
    itemSources[ItemSourceCode::GROUND_SATURN_STONE] = new ItemOnGround(0x0203AB, "Witch Helga's Hut: Saturn Stone on ground");
    itemSources[ItemSourceCode::GROUND_VENUS_STONE] = new ItemOnGround(0x01F8D7, "King Nole's Labyrinth (-3F): Venus Stone on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_1] = new ItemOnGround(0x01E873, "Lake Shrine (-2F): north EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_2] = new ItemOnGround(0x01E87B, "Lake Shrine (-2F): south EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_3] = new ItemOnGround(0x01E883, "Lake Shrine (-2F): west EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_4] = new ItemOnGround(0x01E88B, "Lake Shrine (-2F): east EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_ICE_SWORD] = new ItemOnGround({ 0x01EE37, 0x01EE41 }, "Lake Shrine (-3F): Sword of Ice on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_1] = new ItemOnGround(0x02011D, "Twinkle Village: first EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_2] = new ItemOnGround(0x020115, "Twinkle Village: second EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_3] = new ItemOnGround(0x02010D, "Twinkle Village: third EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_EKEEKE] = new ItemOnGround(0x0226A3, "Mir Tower: EkeEke on ground in priest room");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_DETOX] = new ItemOnGround(0x02269B, "Mir Tower: Detox Grass on ground in priest room");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_RECORD_BOOK] = new ItemOnGround(0x022673, "Mir Tower: Record Book on ground in priest room");
    itemSources[ItemSourceCode::GROUND_LOGS_1] = new ItemOnGround(0x01FA43, "King Nole's Labyrinth (-2F): left Logs on ground");
    itemSources[ItemSourceCode::GROUND_LOGS_2] = new ItemOnGround(0x01FA3B, "King Nole's Labyrinth (-2F): right Logs on ground");
    itemSources[ItemSourceCode::GROUND_FIREDEMON_EKEEKE] = new ItemOnGround(0x01F8E1, "King Nole's Labyrinth (-3F): EkeEke on ground before Firedemon");

    // This ground item is special in the sense that it can only be taken once, so we take advantage of this to consider it as a shop.
    // It will allow putting special items usually not allowed on ground (e.g. Lifestock) inside.
    itemSources[ItemSourceCode::GROUND_FALLING_RIBBON] = new ItemInShop(0x01BFDF, "Mercator: falling ribbon in castle court", nullptr);
}

void World::initShops()
{
    ItemShop* massanShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MASSAN_LIFESTOCK] = new ItemInShop(0x02101D, "Massan shop: Life Stock slot", massanShop);
    itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_1] = new ItemInShop(0x021015, "Massan shop: first EkeEke slot", massanShop);
    itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_2] = new ItemInShop(0x02100D, "Massan shop: second EkeEke slot", massanShop);
    shops.push_back(massanShop);

    ItemShop* gumiInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_GUMI_LIFESTOCK] = new ItemInShop(0x0211E5, "Gumi shop: Life Stock slot", gumiInn);
    itemSources[ItemSourceCode::SHOP_GUMI_EKEEKE] = new ItemInShop(0x0211D5, "Gumi shop: EkeEke slot", gumiInn);
    shops.push_back(gumiInn);

    ItemShop* ryumaShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_RYUMA_LIFESTOCK] = new ItemInShop(0x0212D9, "Ryuma shop: Life Stock slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_GAIA_STATUE] = new ItemInShop(0x0212C9, "Ryuma shop: Statue of Gaia slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_GOLDEN_STATUE] = new ItemInShop(0x0212B9, "Ryuma shop: Golden Statue slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_EKEEKE] = new ItemInShop(0x0212D1, "Ryuma shop: EkeEke slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_DETOX_GRASS] = new ItemInShop(0x0212C1, "Ryuma shop: Detox Grass slot", ryumaShop);
    shops.push_back(ryumaShop);

    ItemShop* ryumaInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_RYUMA_INN_EKEEKE] = new ItemInShop(0x02139F, "Ryuma inn: EkeEke slot", ryumaInn);
    shops.push_back(ryumaInn);

    ItemShop* mercatorShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_ARMOR] = new ItemInShop(0x021B7B, "Mercator shop: Steel Breast slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_BELL] = new ItemInShop(0x021B83, "Mercator shop: Bell slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_EKEEKE] = new ItemInShop(0x021B73, "Mercator shop: EkeEke slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DETOX_GRASS] = new ItemInShop(0x021B6B, "Mercator shop: Detox Grass slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_GAIA_STATUE] = new ItemInShop(0x021B63, "Mercator shop: Statue of Gaia slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_GOLDEN_STATUE] = new ItemInShop(0x021B5B, "Mercator shop: Golden Statue slot", mercatorShop);
    shops.push_back(mercatorShop);

    ItemShop* mercatorDocksShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_1] = new ItemInShop({ 0x0216BD, 0x02168B }, "Mercator docks shop: left EkeEke slot", mercatorDocksShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_2] = new ItemInShop({ 0x0216C5, 0x021693 }, "Mercator docks shop: middle EkeEke slot", mercatorDocksShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_3] = new ItemInShop({ 0x0216CD, 0x02169B }, "Mercator docks shop: right EkeEke slot", mercatorDocksShop);
    shops.push_back(mercatorDocksShop);

    ItemShop* mercatorSpecialShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_MIND_REPAIR] = new ItemInShop(0x021CDF, "Mercator special shop: Mind Repair slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_ANTIPARALYZE] = new ItemInShop(0x021CD7, "Mercator special shop: Anti Paralyze slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_DAHL] = new ItemInShop(0x021CCF, "Mercator special shop: Dahl slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_RESTORATION] = new ItemInShop(0x021CC7, "Mercator special shop: Restoration slot", mercatorSpecialShop);
    shops.push_back(mercatorSpecialShop);

    ItemShop* verlaShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_VERLA_LIFESTOCK] = new ItemInShop(0x021F57, "Verla shop: Life Stock slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_EKEEKE] = new ItemInShop(0x021F37, "Verla shop: EkeEke slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_DETOX_GRASS] = new ItemInShop(0x021F3F, "Verla shop: Detox Grass slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_DAHL] = new ItemInShop(0x021F47, "Verla shop: Dahl slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_MAP] = new ItemInShop(0x021F4F, "Verla shop: Map slot", verlaShop);
    shops.push_back(verlaShop);

    ItemShop* kelketoShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_KELKETO_LIFESTOCK] = new ItemInShop(0x020861, "Kelketo Waterfalls shop: Life Stock slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_EKEEKE] = new ItemInShop(0x020869, "Kelketo Waterfalls shop: EkeEke slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_DETOX_GRASS] = new ItemInShop(0x020871, "Kelketo Waterfalls shop: Detox Grass slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_DAHL] = new ItemInShop(0x020879, "Kelketo Waterfalls shop: Dahl slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_RESTORATION] = new ItemInShop(0x020881, "Kelketo Waterfalls shop: Restoration slot", kelketoShop);
    shops.push_back(kelketoShop);

    ItemShop* destelInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_DESTEL_INN_EKEEKE] = new ItemInShop(0x022017, "Destel inn: EkeEke slot", destelInn);
    shops.push_back(destelInn);

    ItemShop* destelShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_DESTEL_EKEEKE] = new ItemInShop(0x022055, "Destel shop: EkeEke slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_DETOX_GRASS] = new ItemInShop(0x02206D, "Destel shop: Detox Grass slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_RESTORATION] = new ItemInShop(0x022065, "Destel shop: Restoration slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_DAHL] = new ItemInShop(0x02205D, "Destel shop: Dahl slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_LIFE_STOCK] = new ItemInShop(0x022075, "Destel shop: Life Stock slot", destelShop);
    shops.push_back(destelShop);

    ItemShop* greedlyShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_GREEDLY_GAIA_STATUE] = new ItemInShop(0x0209C7, "Greedly's shop: Statue of Gaia slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_GOLDEN_STATUE] = new ItemInShop(0x0209BF, "Greedly's shop: Golden Statue slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_DAHL] = new ItemInShop(0x0209CF, "Greedly's shop: Dahl slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_LIFE_STOCK] = new ItemInShop(0x0209AF, "Greedly's shop: Life Stock slot", greedlyShop);
    shops.push_back(greedlyShop);

    ItemShop* kazaltShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_KAZALT_EKEEKE] = new ItemInShop(0x022115, "Kazalt shop: EkeEke slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_DAHL] = new ItemInShop(0x022105, "Kazalt shop: Dahl slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_GOLDEN_STATUE] = new ItemInShop(0x02211D, "Kazalt shop: Golden Statue slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_RESTORATION] = new ItemInShop(0x02210D, "Kazalt shop: Restoration slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_LIFESTOCK] = new ItemInShop(0x0220F5, "Kazalt shop: Life Stock slot", kazaltShop);
    shops.push_back(kazaltShop);
}

void World::initNPCRewards()
{
    itemSources[ItemSourceCode::NPC_MIR_AXE_MAGIC] = new ItemReward(0x028A3F, "Mir reward after Lake Shrine (Axe Magic in OG)");
    itemSources[ItemSourceCode::NPC_ZAK_GOLA_EYE] = new ItemReward(0x028A73, "Zak reward after fighting (Gola's Eye in OG)");
    itemSources[ItemSourceCode::NPC_KADO_MAGIC_SWORD] = new ItemReward(0x02894B, "Swordman Kado reward (Magic Sword in OG)");
    itemSources[ItemSourceCode::NPC_HIDDEN_DWARF_RESTORATION] = new ItemReward(0x0288DF, "Greenmaze hidden dwarf (Restoration in OG)");
}

void World::initRegions()
{
    regions[RegionCode::MASSAN] = new WorldRegion("Massan", { 
        itemSources[ItemSourceCode::CHEST_MASSAN_DOG_STATUE],
        itemSources[ItemSourceCode::CHEST_MASSAN_HOUSE_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_1],
        itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_2],
        itemSources[ItemSourceCode::SHOP_MASSAN_LIFESTOCK],
        itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_1],
        itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_2]
    });
    regions[RegionCode::MASSAN]->addItemSource(itemSources[ItemSourceCode::CHEST_MASSAN_FARA_REWARD], items[ITEM_IDOL_STONE]);

    regions[RegionCode::MASSAN_CAVE] = new WorldRegion("Massan Cave", { 
        itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_DAHL],
        itemSources[ItemSourceCode::GROUND_FIREPROOF]
    });

    regions[RegionCode::ROUTE_MASSAN_GUMI] = new WorldRegion("Route between Massan and Gumi", { 
        itemSources[ItemSourceCode::CHEST_ROUTE_MASSAN_GUMI_PROMONTORY]
    });

    regions[RegionCode::WATERFALL_SHRINE] = new WorldRegion("Waterfall Shrine", { 
        itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_ENTRANCE_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_END_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_KEY],
        itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_GOLDS]
    });

    regions[RegionCode::SWAMP_SHRINE] = new WorldRegion("Swamp Shrine", { 
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FRONT_EKEEKE],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_RIGHT_EKEEKE],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FALLING_EKEEKE],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_CARPET_KEY],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_LOWER_BRIDGES_KEY],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_UPPER_BRIDGES_KEY],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_2F_SPIKE_KEY],
        itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_3F_REWARD]
    });

    regions[RegionCode::GUMI] = new WorldRegion("Gumi", { 
        itemSources[ItemSourceCode::CHEST_GUMI_LIFESTOCK],
        itemSources[ItemSourceCode::GROUND_IDOL_STONE],
        itemSources[ItemSourceCode::SHOP_GUMI_LIFESTOCK],
        itemSources[ItemSourceCode::SHOP_GUMI_EKEEKE]
    });
    regions[RegionCode::GUMI]->addItemSource(itemSources[ItemSourceCode::CHEST_GUMI_FARA_REWARD], items[ITEM_IDOL_STONE]);

    regions[RegionCode::ROUTE_GUMI_RYUMA] = new WorldRegion("Route from Gumi to Ryuma", { 
        itemSources[ItemSourceCode::CHEST_ROUTE_GUMI_RYUMA_LIFESTOCK],
        itemSources[ItemSourceCode::NPC_KADO_MAGIC_SWORD]
    });

    regions[RegionCode::TIBOR] = new WorldRegion("Tibor", { 
        itemSources[ItemSourceCode::CHEST_TIBOR_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_TIBOR_SPIKEBALLS_ROOM],
        itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_LEFT],
        itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_RIGHT],
    });

    regions[RegionCode::RYUMA] = new WorldRegion("Ryuma", { 
        itemSources[ItemSourceCode::CHEST_RYUMA_MAYOR_LIFESTOCK],
        itemSources[ItemSourceCode::SHOP_RYUMA_LIFESTOCK],
        itemSources[ItemSourceCode::SHOP_RYUMA_GAIA_STATUE],
        itemSources[ItemSourceCode::SHOP_RYUMA_GOLDEN_STATUE],
        itemSources[ItemSourceCode::SHOP_RYUMA_EKEEKE],
        itemSources[ItemSourceCode::SHOP_RYUMA_DETOX_GRASS],
        itemSources[ItemSourceCode::SHOP_RYUMA_INN_EKEEKE]
    });
    regions[RegionCode::RYUMA]->addItemSource(itemSources[ItemSourceCode::CHEST_RYUMA_LIGHTHOUSE_LIFESTOCK], items[ITEM_SUN_STONE]);

    regions[RegionCode::THIEVES_HIDEOUT] = new WorldRegion("Thieves Hideout", { 
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_EKEEKE_ROLLING_BOULDER],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_ENTRANCE_EKEEKE],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_FIRST_PLATFORM_ROOM],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_SECOND_PLATFORM_ROOM],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MOVING_BALLS_RIDDLE],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LITHOGRAPH],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_5G],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_50G],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_EKEEKE]
    });

    regions[RegionCode::WITCH_HELGA_HUT] = new WorldRegion("Witch Helga's Hut", { 
        itemSources[ItemSourceCode::CHEST_HELGA],
        itemSources[ItemSourceCode::GROUND_SATURN_STONE]
    });

    regions[RegionCode::MERCATOR] = new WorldRegion("Mercator", { 
        itemSources[ItemSourceCode::CHEST_MERCATOR_CASTLE_KITCHEN],
        itemSources[ItemSourceCode::CHEST_MERCATOR_ARTHUR_KEY],
        itemSources[ItemSourceCode::CHEST_MERCATOR_GREENPEA],
        itemSources[ItemSourceCode::CHEST_MERCATOR_GRANDMA_POT_SHELVING],
        itemSources[ItemSourceCode::CHEST_MERCATOR_SICK_MERCHANT],
        itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO],
        itemSources[ItemSourceCode::GROUND_FALLING_RIBBON],
        itemSources[ItemSourceCode::SHOP_MERCATOR_ARMOR],
        itemSources[ItemSourceCode::SHOP_MERCATOR_BELL],
        itemSources[ItemSourceCode::SHOP_MERCATOR_EKEEKE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DETOX_GRASS],
        itemSources[ItemSourceCode::SHOP_MERCATOR_GAIA_STATUE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_GOLDEN_STATUE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_1],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_2],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_3]
    });

    regions[RegionCode::MERCATOR_SPECIAL_SHOP] = new WorldRegion("Mercator special shop", { 
        itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_MIND_REPAIR],
        itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_ANTIPARALYZE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_DAHL],
        itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_RESTORATION]
    });

    regions[RegionCode::CRYPT] = new WorldRegion("Crypt", { 
        itemSources[ItemSourceCode::CHEST_CRYPT_MAIN_LOBBY],
        itemSources[ItemSourceCode::CHEST_CRYPT_ARMLET]
    });

    regions[RegionCode::MERCATOR_DUNGEON] = new WorldRegion("Mercator Dungeon", { 
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_KEY],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_LEFT],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_RIGHT],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_LEFT],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_RIGHT],
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_LIFESTOCK]
    });

    regions[RegionCode::MIR_TOWER_SECTOR] = new WorldRegion("Mir Tower sector", {
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_GOLDEN_STATUE],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_RESTORATION],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TWINKLE_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_HIDDEN_BUTTON_LIFESTOCK],
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_1],
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_2],
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_3]
    });
    regions[RegionCode::MIR_TOWER_SECTOR]->addItemSource(itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_DAHL], items[ITEM_AXE_MAGIC]);
    regions[RegionCode::MIR_TOWER_SECTOR]->addItemSource(itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_LIFESTOCK], items[ITEM_AXE_MAGIC]);

    regions[RegionCode::MIR_TOWER_PRE_GARLIC] = new WorldRegion("Mir Tower (pre-garlic)", {
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_MUSHROOM_PIT_ROOM],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_GAIA_STATUE],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_1],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_2],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_3],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_4],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_GARLIC],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_AFTER_MIMICS]
    });

    regions[RegionCode::MIR_TOWER_POST_GARLIC] = new WorldRegion("Mir Tower (post-garlic)", {
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_LIBRARY_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_HIDDEN_EKEEKE],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_FALLING_SPIKEBALLS],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_TIMED_KEY],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_1],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_2],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_PURPLE_JEWEL],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LEFT_EKEEKE],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_RIGHT_EKEEKE],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LIFESTOCK],
        itemSources[ItemSourceCode::GROUND_MIR_TOWER_EKEEKE],
        itemSources[ItemSourceCode::GROUND_MIR_TOWER_DETOX],
        itemSources[ItemSourceCode::GROUND_MIR_TOWER_RECORD_BOOK]
    });

    regions[RegionCode::GREENMAZE] = new WorldRegion("Greenmaze", {
        itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_WHISTLE],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_PROMONTORY_GOLDS],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_MAGES_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_LEFT],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_RIGHT],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_DAHL],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_GOLDS],
        itemSources[ItemSourceCode::NPC_HIDDEN_DWARF_RESTORATION]
    });

    regions[RegionCode::GREENMAZE_BEHIND_TREES] = new WorldRegion("Greenmaze (behind sacred trees)", {
        itemSources[ItemSourceCode::CHEST_MASSAN_SHORTCUT_DAHL],
        itemSources[ItemSourceCode::GROUND_SUN_STONE]
    });

    regions[RegionCode::VERLA_SECTOR] = new WorldRegion("Verla sector", {
	    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_BEHIND_CABIN],
	    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_ANGLE_PROMONTORY],
	    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_CLIFF_CHEST],
	    itemSources[ItemSourceCode::SHOP_VERLA_LIFESTOCK],
	    itemSources[ItemSourceCode::SHOP_VERLA_EKEEKE],
	    itemSources[ItemSourceCode::SHOP_VERLA_DETOX_GRASS],
	    itemSources[ItemSourceCode::SHOP_VERLA_DAHL],
        itemSources[ItemSourceCode::SHOP_VERLA_MAP]
    });

    regions[RegionCode::VERLA_MINES] = new WorldRegion("Verla Mines", {
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_LEFT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_RIGHT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_JAR_STAIRCASE_ROOM],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_DEX_KEY],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_SLASHER_KEY],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_LEFT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_MIDDLE],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_RIGHT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_RIGHT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_LEFT],
	    itemSources[ItemSourceCode::CHEST_VERLA_MINES_LAVA_WALKING],
	    itemSources[ItemSourceCode::CHEST_VERLA_WELL],
	    itemSources[ItemSourceCode::GROUND_CHROME_BREAST]
    });

    regions[RegionCode::ROUTE_VERLA_DESTEL] = new WorldRegion("Route between Verla and Destel", {
	    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_BUSHES_DAHL],
	    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_ELEVATOR],
	    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_HIDDEN_LIFESTOCK],
	    itemSources[ItemSourceCode::SHOP_KELKETO_LIFESTOCK],
	    itemSources[ItemSourceCode::SHOP_KELKETO_EKEEKE],
	    itemSources[ItemSourceCode::SHOP_KELKETO_DETOX_GRASS],
	    itemSources[ItemSourceCode::SHOP_KELKETO_DAHL],
	    itemSources[ItemSourceCode::SHOP_KELKETO_RESTORATION]
    });

    regions[RegionCode::DESTEL] = new WorldRegion("Destel", {
	    itemSources[ItemSourceCode::CHEST_DESTEL_INN_COUNTER],
	    itemSources[ItemSourceCode::SHOP_DESTEL_INN_EKEEKE],
	    itemSources[ItemSourceCode::SHOP_DESTEL_EKEEKE],
	    itemSources[ItemSourceCode::SHOP_DESTEL_DETOX_GRASS],
	    itemSources[ItemSourceCode::SHOP_DESTEL_RESTORATION],
	    itemSources[ItemSourceCode::SHOP_DESTEL_DAHL],
	    itemSources[ItemSourceCode::SHOP_DESTEL_LIFE_STOCK]
    });

    regions[RegionCode::ROUTE_AFTER_DESTEL] = new WorldRegion("Route after Destel", {
	    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_CORNER_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_HIDDEN_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_DAHL],
	    itemSources[ItemSourceCode::GROUND_MARS_STONE]
    });

    regions[RegionCode::DESTEL_WELL] = new WorldRegion("Destel Well", {
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_CRATES_KEY],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_STAIRS_EKEEKE],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_NARROW_GROUND_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_SPIKE_HALLWAY],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_SIDE_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_DAHL],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_RIGHT],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_LEFT],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_1],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_2],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_3],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_TOP],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_LEFT],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_DOWN],
	    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_RIGHT],
	    itemSources[ItemSourceCode::GROUND_HEALING_BOOTS]
    });

    regions[RegionCode::ROUTE_LAKE_SHRINE] = new WorldRegion("Route to Lake Shrine", {
	    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_EASY_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_LEFT],
	    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_RIGHT],
	    itemSources[ItemSourceCode::SHOP_GREEDLY_GAIA_STATUE],
	    itemSources[ItemSourceCode::SHOP_GREEDLY_GOLDEN_STATUE],
	    itemSources[ItemSourceCode::SHOP_GREEDLY_DAHL],
	    itemSources[ItemSourceCode::SHOP_GREEDLY_LIFE_STOCK]
    });

    regions[RegionCode::LAKE_SHRINE] = new WorldRegion("Lake Shrine", {
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_KEY],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLDEN_STATUE],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLEM_HOPPING_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_LIFESTOCK_FALLING_FROM_UNICORNS],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_KEY],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_WHITE_GOLEMS_CEILING],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_KEY_NEAR_SWORD],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_SNAKE_CAGING_RIDDLE],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_LIFESTOCK_FALLING_FROM_PLATFORMS],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_HALLWAY_TO_DUKE],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_LEFT],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_MIDDLE],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_RIGHT],
	    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_GOLDEN_SPINNER_KEY],
	    itemSources[ItemSourceCode::GROUND_SHELL_BREAST],
	    itemSources[ItemSourceCode::GROUND_ICE_SWORD],
	    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_1],
	    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_2],
	    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_3],
	    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_4],
	    itemSources[ItemSourceCode::NPC_MIR_AXE_MAGIC]
    });

    regions[RegionCode::MOUNTAINOUS_AREA] = new WorldRegion("Mountainous Area", {
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BELOW_ROCKY_ARCH],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_DAHL_NEAR_TREE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LIFESTOCK_BEFORE_BRIDGE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GOLDEN_STATUE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GAIA_STATUE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_CLIFF_LIFESTOCK],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_LEFT],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_RIGHT],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_RESTORATION_NEAR_ZAK],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_ZAK],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_BRIDGE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_VISIBLE],
        itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LAKE_SHRINE_SHORTCUT],
        itemSources[ItemSourceCode::GROUND_MOON_STONE],
        itemSources[ItemSourceCode::NPC_ZAK_GOLA_EYE]
    });

    regions[RegionCode::KN_CAVE] = new WorldRegion("King Nole's Cave", {
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_GOLD],
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_GOLD],
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_THIRD_GOLD],
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_KN_CAVE_BOULDER_CHASE_LIFESTOCK]
    });

    regions[RegionCode::KAZALT] = new WorldRegion("Kazalt", {
	    itemSources[ItemSourceCode::SHOP_KAZALT_EKEEKE],
	    itemSources[ItemSourceCode::SHOP_KAZALT_DAHL],
	    itemSources[ItemSourceCode::SHOP_KAZALT_GOLDEN_STATUE],
	    itemSources[ItemSourceCode::SHOP_KAZALT_RESTORATION],
        itemSources[ItemSourceCode::SHOP_KAZALT_LIFESTOCK]
    });

    regions[RegionCode::KN_LABYRINTH_PRE_SPIKES] = new WorldRegion("King Nole's Labyrinth (pre-spikes)", {
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EXTERIOR_KEY],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EKEEKE_AFTER_KEYDOOR],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_LIFESTOCK_AFTER_KEYDOOR],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SMALL_MAZE_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SPIKE_BALLS_GAIA_STATUE],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_1],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_2],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_3],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_BIG_MAZE_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN_ROOM_EKEEKE],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_ICE_SHORTCUT_KEY],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_SAVE_ROOM],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_BUTTON_CRATES_KEY],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_EKEEKE],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_DAHL],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_REWARD],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FOUR_BUTTONS_RIDDLE],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_DEVIL_HANDS],
        itemSources[ItemSourceCode::GROUND_FIREDEMON_EKEEKE],
	    itemSources[ItemSourceCode::GROUND_SPIKE_BOOTS],
	    itemSources[ItemSourceCode::GROUND_IRON_BOOTS],
	    itemSources[ItemSourceCode::GROUND_GAIA_SWORD],
	    itemSources[ItemSourceCode::GROUND_VENUS_STONE]
    });

    regions[RegionCode::KN_LABYRINTH_POST_SPIKES] = new WorldRegion("King Nole's Labyrinth (post-spikes)", {
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_GARLIC],
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_REWARD]
    });
    regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addItemSource(itemSources[ItemSourceCode::GROUND_LOGS_1], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addItemSource(itemSources[ItemSourceCode::GROUND_LOGS_2], items[ITEM_AXE_MAGIC]);

    regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR] = new WorldRegion("King Nole's Labyrinth (raft sector)", {
        itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_1],
        itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_2],
        itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_GAIA_STATUE],    // content fixed to "Logs" during randomization
        itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_REWARD],
        itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_KEY],
        itemSources[ItemSourceCode::GROUND_HYPER_BREAST]
    });

    regions[RegionCode::KN_PALACE] = new WorldRegion("King Nole's Palace", {
	    itemSources[ItemSourceCode::CHEST_KN_PALACE_LIFESTOCK],
	    itemSources[ItemSourceCode::CHEST_KN_PALACE_EKEEKE],
	    itemSources[ItemSourceCode::CHEST_KN_PALACE_DAHL]
    });

    // Create a fake region to require the 3 Gola items
    regions[RegionCode::ENDGAME] = new WorldRegion("The End", {});
}

void World::initRegionPaths()
{
	regions[RegionCode::MASSAN]->addPathTo(regions[RegionCode::MASSAN_CAVE], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::MASSAN]->addPathTo(regions[RegionCode::ROUTE_MASSAN_GUMI]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathTo(regions[RegionCode::WATERFALL_SHRINE]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathTo(regions[RegionCode::SWAMP_SHRINE], items[ITEM_IDOL_STONE]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathTo(regions[RegionCode::GUMI]);
	regions[RegionCode::GUMI]->addPathTo(regions[RegionCode::ROUTE_GUMI_RYUMA]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathTo(regions[RegionCode::TIBOR]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathTo(regions[RegionCode::RYUMA]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathTo(regions[RegionCode::MERCATOR], items[ITEM_SAFETY_PASS]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathTo(regions[RegionCode::WITCH_HELGA_HUT], items[ITEM_EINSTEIN_WHISTLE]);
	regions[RegionCode::RYUMA]->addPathTo(regions[RegionCode::THIEVES_HIDEOUT]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::MERCATOR_DUNGEON]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::CRYPT]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::MIR_TOWER_SECTOR]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::MERCATOR_SPECIAL_SHOP], items[ITEM_BUYER_CARD]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::GREENMAZE], items[ITEM_KEY]);
	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::VERLA_SECTOR], items[ITEM_SUN_STONE]);
	regions[RegionCode::MIR_TOWER_SECTOR]->addPathTo(regions[RegionCode::MIR_TOWER_PRE_GARLIC], items[ITEM_ARMLET]);
	regions[RegionCode::MIR_TOWER_PRE_GARLIC]->addPathTo(regions[RegionCode::MIR_TOWER_POST_GARLIC], items[ITEM_GARLIC]);
	regions[RegionCode::VERLA_SECTOR]->addPathTo(regions[RegionCode::VERLA_MINES]);
	regions[RegionCode::VERLA_MINES]->addPathTo(regions[RegionCode::ROUTE_VERLA_DESTEL]);
	regions[RegionCode::ROUTE_VERLA_DESTEL]->addPathTo(regions[RegionCode::DESTEL]);
	regions[RegionCode::DESTEL]->addPathTo(regions[RegionCode::ROUTE_AFTER_DESTEL]);
	regions[RegionCode::DESTEL]->addPathTo(regions[RegionCode::DESTEL_WELL]);
	regions[RegionCode::DESTEL_WELL]->addPathTo(regions[RegionCode::ROUTE_LAKE_SHRINE]);
	regions[RegionCode::ROUTE_LAKE_SHRINE]->addPathTo(regions[RegionCode::LAKE_SHRINE], items[ITEM_GAIA_STATUE]);
	regions[RegionCode::GREENMAZE]->addPathTo(regions[RegionCode::MOUNTAINOUS_AREA], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::GREENMAZE]->addPathTo(regions[RegionCode::GREENMAZE_BEHIND_TREES], items[ITEM_EINSTEIN_WHISTLE]);
	regions[RegionCode::MOUNTAINOUS_AREA]->addPathTo(regions[RegionCode::ROUTE_LAKE_SHRINE], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::MOUNTAINOUS_AREA]->addPathTo(regions[RegionCode::KN_CAVE], items[ITEM_GOLA_EYE]);
	regions[RegionCode::KN_CAVE]->addPathTo(regions[RegionCode::KAZALT]);
	regions[RegionCode::KAZALT]->addPathTo(regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]);
	regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]->addPathTo(regions[RegionCode::KN_LABYRINTH_POST_SPIKES], items[ITEM_SPIKE_BOOTS]);
	regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addPathTo(regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR], items[ITEM_LOGS]);
	regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addPathTo(regions[RegionCode::KN_PALACE]);
	regions[RegionCode::KN_PALACE]->addPathTo(regions[RegionCode::ENDGAME], { items[ITEM_GOLA_FANG], items[ITEM_GOLA_HORN],  items[ITEM_GOLA_NAIL] });
}

void World::initTreeMaps()
{
    treeMaps = {
        TreeMap("Massan sector", 0x11DC56, 0x11DC66, 0x0200),
        TreeMap("Tibor sector", 0x11DD9E, 0x11DDA6, 0x0216),
        TreeMap("Mercator front gate", 0x11DDD6, 0x11DDDE, 0x021B),
        TreeMap("Verla shore", 0x11DEA6, 0x11DEAE, 0x0219),
        TreeMap("Destel sector", 0x11DFA6, 0x11DFAE, 0x0218),
        TreeMap("Lake Shrine sector", 0x11DFE6, 0x11DFEE, 0x0201),
        TreeMap("Mir Tower sector", 0x11DE06, 0x11DE0E, 0x021A),
        TreeMap("Mountainous area", 0x11DF3E, 0x11DF46, 0x0217),
        TreeMap("Greenmaze entrance", 0x11E1DE, 0x11E1E6, 0x01FF),
        TreeMap("Greenmaze exit", 0x11E25E, 0x11E266, 0x01FE)
    };
}