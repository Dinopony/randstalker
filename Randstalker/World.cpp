#include "World.h"
#include <algorithm>

void World::initItems()
{
	_items[ITEM_EKEEKE] = new Item(ITEM_EKEEKE, "EkeEke", 20, true);
	_items[ITEM_MAGIC_SWORD] = new Item(ITEM_MAGIC_SWORD, "Magic Sword", 200, true);
	_items[ITEM_ICE_SWORD] = new Item(ITEM_ICE_SWORD, "Sword of Ice", 300, true);
	_items[ITEM_THUNDER_SWORD] = new Item(ITEM_THUNDER_SWORD, "Thunder Sword", 400, true);
	_items[ITEM_GAIA_SWORD] = new Item(ITEM_GAIA_SWORD, "Sword of Gaia", 500, true);
	_items[ITEM_FIREPROOF_BOOTS] = new Item(ITEM_FIREPROOF_BOOTS, "Fireproof", 100, true);
	_items[ITEM_IRON_BOOTS] = new Item(ITEM_IRON_BOOTS, "Iron Boots", 200, true);
	_items[ITEM_HEALING_BOOTS] = new Item(ITEM_HEALING_BOOTS, "Healing Boots", 500, true);
	_items[ITEM_SPIKE_BOOTS] = new Item(ITEM_SPIKE_BOOTS, "Snow Spikes", 300, true);
	_items[ITEM_STEEL_BREAST] = new Item(ITEM_STEEL_BREAST, "Steel Breast", 300, true);
	_items[ITEM_CHROME_BREAST] = new Item(ITEM_CHROME_BREAST, "Chrome Breast", 400, true);
	_items[ITEM_SHELL_BREAST] = new Item(ITEM_SHELL_BREAST, "Shell Breast", 500, true);
	_items[ITEM_HYPER_BREAST] = new Item(ITEM_HYPER_BREAST, "Hyper Breast", 750, true);
	_items[ITEM_MARS_STONE] = new Item(ITEM_MARS_STONE, "Mars Stone", 100, true);
	_items[ITEM_MOON_STONE] = new Item(ITEM_MOON_STONE, "Moon Stone", 150, true);
	_items[ITEM_SATURN_STONE] = new Item(ITEM_SATURN_STONE, "Saturn Stone", 200, true);
	_items[ITEM_VENUS_STONE] = new Item(ITEM_VENUS_STONE, "Venus Stone", 300, true);
	_items[ITEM_DETOX_GRASS] = new Item(ITEM_DETOX_GRASS, "Detox Grass", 20, true);
	_items[ITEM_GAIA_STATUE] = new Item(ITEM_GAIA_STATUE, "Statue of Gaia", 150);
	_items[ITEM_GOLDEN_STATUE] = new Item(ITEM_GOLDEN_STATUE, "Golden Statue", 200);
	_items[ITEM_MIND_REPAIR] = new Item(ITEM_MIND_REPAIR, "Mind Repair", 20, true);
	_items[ITEM_CASINO_TICKET] = new Item(ITEM_CASINO_TICKET, "Casino Ticket", 1); // What to do with them?
	_items[ITEM_AXE_MAGIC] = new Item(ITEM_AXE_MAGIC, "Axe Magic", 400, true);
	_items[ITEM_BLUE_RIBBON] = new Item(ITEM_BLUE_RIBBON, "Blue Ribbon", 1, true); // What to do with it?
	_items[ITEM_BUYER_CARD] = new Item(ITEM_BUYER_CARD, "Buyer's Card", 100, true); // What to do with it?
	_items[ITEM_LANTERN] = new Item(ITEM_LANTERN, "Lantern", 150, true);
	_items[ITEM_GARLIC] = new Item(ITEM_GARLIC, "Garlic", 150, true);
	_items[ITEM_ANTI_PARALYZE] = new Item(ITEM_ANTI_PARALYZE, "Anti Paralyze", 20);
	_items[ITEM_STATUE_JYPTA] = new Item(ITEM_STATUE_JYPTA, "Statue of Jypta", 2000, true); // What to do with it?
	_items[ITEM_SUN_STONE] = new Item(ITEM_SUN_STONE, "Sun Stone", 400, true);
	_items[ITEM_ARMLET] = new Item(ITEM_ARMLET, "Armlet", 300, true);
	_items[ITEM_EINSTEIN_WHISTLE] = new Item(ITEM_EINSTEIN_WHISTLE, "Einstein Whistle", 300, true);
	_items[ITEM_SPELL_BOOK] = new Item(ITEM_SPELL_BOOK, "Spell Book", 50, true); // What to do with it?
	_items[ITEM_LITHOGRAPH] = new Item(ITEM_LITHOGRAPH, "Lithograph", 100, true); // What to do with it?
	_items[ITEM_RED_JEWEL] = new Item(ITEM_RED_JEWEL, "Red Jewel", 100, true); // What to do with it?
	_items[ITEM_PAWN_TICKET] = new Item(ITEM_PAWN_TICKET, "Pawn Ticket", 100, true);
	_items[ITEM_PURPLE_JEWEL] = new Item(ITEM_PURPLE_JEWEL, "Purple Jewel", 1); // What to do with it?
	_items[ITEM_GOLA_EYE] = new Item(ITEM_GOLA_EYE, "Gola's Eye", 500, true);
	_items[ITEM_DEATH_STATUE] = new Item(ITEM_DEATH_STATUE, "Death Statue", 150, true);
	_items[ITEM_DAHL] = new Item(ITEM_DAHL, "Dahl", 150);
	_items[ITEM_RESTORATION] = new Item(ITEM_RESTORATION, "Restoration", 40, true);
	_items[ITEM_LOGS] = new Item(ITEM_LOGS, "Logs", 100, true);
	_items[ITEM_ORACLE_STONE] = new Item(ITEM_ORACLE_STONE, "Oracle Stone", 100, true); // What to do with it?
	_items[ITEM_IDOL_STONE] = new Item(ITEM_IDOL_STONE, "Idol Stone", 200, true);
	_items[ITEM_KEY] = new Item(ITEM_KEY, "Key", 150, true);
	_items[ITEM_SAFETY_PASS] = new Item(ITEM_SAFETY_PASS, "Safety Pass", 300, true);
	_items[ITEM_BELL] = new Item(ITEM_BELL, "Bell", 300, true);
	_items[ITEM_SHORT_CAKE] = new Item(ITEM_SHORT_CAKE, "Short Cake", 100, true); // What to do with it?
	_items[ITEM_GOLA_NAIL] = new Item(ITEM_GOLA_NAIL, "Gola's Nail", 1000, true);
	_items[ITEM_GOLA_HORN] = new Item(ITEM_GOLA_HORN, "Gola's Horn", 1000, true);
	_items[ITEM_GOLA_FANG] = new Item(ITEM_GOLA_FANG, "Gola's Fang", 1000, true);
	_items[ITEM_5_GOLDS] = new Item(ITEM_5_GOLDS, "5 golds", 0);
	_items[ITEM_20_GOLDS] = new Item(ITEM_20_GOLDS, "20 golds", 0);
	_items[ITEM_50_GOLDS] = new Item(ITEM_50_GOLDS, "50 golds", 0);
	_items[ITEM_200_GOLDS] = new Item(ITEM_200_GOLDS, "200 golds", 0);
	_items[ITEM_LIFESTOCK] = new Item(ITEM_LIFESTOCK, "Life Stock", 300);
	_items[ITEM_NONE] = new Item(ITEM_NONE, "Empty", 0);

	// The following items are placeholder items, thus they are missing from this list:
	//      - Awakening Book (0x11)
	//      - Detox Book (0x21)
	//      - AntiCurse (0x22)
	//      - Record Book (0x23)
	//      - Hotel Register (0x25)
	//      - Island Map (0x26)
	//      - No52 (0x34)
}

void World::initItemSources()
{
	_chests[0x00] = new ItemChest(0x00, ITEM_EKEEKE, "Swamp Shrine (0F): ekeeke chest in room to the right, close to door");
	_chests[0x01] = new ItemChest(0x01, ITEM_KEY, "Swamp Shrine (0F): key chest in carpet room");
	_chests[0x02] = new ItemChest(0x02, ITEM_LIFESTOCK, "Swamp Shrine (0F): lifestock chest in room to the left, accessible by falling from upstairs");
	_chests[0x03] = new ItemChest(0x03, ITEM_EKEEKE, "Swamp Shrine (0F): ekeeke chest falling from the ceiling when beating the orc");
	_chests[0x04] = new ItemChest(0x04, ITEM_EKEEKE, "Swamp Shrine (0F): ekeeke chest in room connected to second entrance (without idol stone)");
	_chests[0x05] = new ItemChest(0x05, ITEM_KEY, "Swamp Shrine (1F): lower key chest in wooden bridges room");
	_chests[0x06] = new ItemChest(0x06, ITEM_KEY, "Swamp Shrine (2F): upper key chest in wooden bridges room");
	_chests[0x07] = new ItemChest(0x07, ITEM_KEY, "Swamp Shrine (1F): key chest in spike room");
	_chests[0x08] = new ItemChest(0x08, ITEM_LIFESTOCK, "Swamp Shrine (2F): lifestock chest in Fara's room");
	_chests[0x09] = new ItemChest(0x09, ITEM_LIFESTOCK, "Mercator Dungeon (-1F): lifestock chest after key door");
	_chests[0x0A] = new ItemChest(0x0A, ITEM_KEY, "Mercator Dungeon (-1F): key chest in Moralis's cell");
	_chests[0x0B] = new ItemChest(0x0B, ITEM_EKEEKE, "Mercator Dungeon (-1F): left ekeeke chest in double chest room");
	_chests[0x0C] = new ItemChest(0x0C, ITEM_EKEEKE, "Mercator Dungeon (-1F): right ekeeke chest in double chest room");
	_chests[0x0D] = new ItemChest(0x0D, ITEM_LIFESTOCK, "Mercator: castle kitchen chest");
	_chests[0x0F] = new ItemChest(0x0F, ITEM_EKEEKE, "Mercator Dungeon (1F): left ekeeke chest in double chest room");
	_chests[0x10] = new ItemChest(0x10, ITEM_EKEEKE, "Mercator Dungeon (1F): right ekeeke chest in double chest room");
	_chests[0x11] = new ItemChest(0x11, ITEM_KEY, "Mercator: Arthur key chest in castle tower");
	_chests[0x12] = new ItemChest(0x12, ITEM_LIFESTOCK, "Mercator Dungeon (4F): chest on top of tower");
	_chests[0x13] = new ItemChest(0x13, ITEM_LIFESTOCK, "King Nole's Palace: entrance lifestock chest");
	_chests[0x14] = new ItemChest(0x14, ITEM_EKEEKE, "King Nole's Palace: ekeeke chest in topmost pit room");
	_chests[0x15] = new ItemChest(0x15, ITEM_DAHL, "King Nole's Palace: dahl chest in floating button room");
	_chests[0x16] = new ItemChest(0x16, ITEM_LIFESTOCK, "King Nole's Cave: first lifestock chest");
	_chests[0x18] = new ItemChest(0x18, ITEM_5_GOLDS, "King Nole's Cave: first gold chest in third room");
	_chests[0x19] = new ItemChest(0x19, ITEM_5_GOLDS, "King Nole's Cave: second gold chest in third room");
	_chests[0x1C] = new ItemChest(0x1C, ITEM_5_GOLDS, "King Nole's Cave: gold chest in isolated room");
	_chests[0x1D] = new ItemChest(0x1D, ITEM_LIFESTOCK, "King Nole's Cave: lifestock chest in crate room");
	_chests[0x1F] = new ItemChest(0x1F, ITEM_LIFESTOCK, "King Nole's Cave: boulder chase corridor chest");
	_chests[0x21] = new ItemChest(0x21, ITEM_LIFESTOCK, "Waterfall Shrine: lifestock chest under entrance (accessible after talking with Prospero)");
	_chests[0x22] = new ItemChest(0x22, ITEM_LIFESTOCK, "Waterfall Shrine: lifestock chest near Prospero");
	_chests[0x23] = new ItemChest(0x23, ITEM_5_GOLDS, "Waterfall Shrine: chest in button room");
	_chests[0x24] = new ItemChest(0x24, ITEM_KEY, "Waterfall Shrine: upstairs key chest");
	_chests[0x26] = new ItemChest(0x26, ITEM_EKEEKE, "Thieves Hideout: chest in entrance room when water is removed");
	_chests[0x29] = new ItemChest(0x29, ITEM_KEY, "Thieves Hideout: right chest in room accessible by falling from miniboss room");
	_chests[0x2A] = new ItemChest(0x2A, ITEM_EKEEKE, "Thieves Hideout: left chest in room accessible by falling from miniboss room");
	_chests[0x2B] = new ItemChest(0x2B, ITEM_KEY, "Thieves Hideout: left chest in Pockets cell");
	_chests[0x2C] = new ItemChest(0x2C, ITEM_LIFESTOCK, "Thieves Hideout: right chest in Pockets cell");
	_chests[0x2D] = new ItemChest(0x2D, ITEM_EKEEKE, "Thieves Hideout: right chest in room after quick climb trial");
	_chests[0x2E] = new ItemChest(0x2E, ITEM_GAIA_STATUE, "Thieves Hideout: left chest in room after quick climb trial");
	_chests[0x2F] = new ItemChest(0x2F, ITEM_EKEEKE, "Thieves Hideout: chest in first platform room");
	_chests[0x30] = new ItemChest(0x30, ITEM_EKEEKE, "Thieves Hideout: chest in second platform room");
	_chests[0x31] = new ItemChest(0x31, ITEM_LIFESTOCK, "Thieves Hideout: reward chest after moving balls room");
	_chests[0x32] = new ItemChest(0x32, ITEM_EKEEKE, "Thieves Hideout: chest near rolling boulder");
	_chests[0x34] = new ItemChest(0x34, ITEM_EKEEKE, "Thieves Hideout: left chest in double ekeeke chest room");
	_chests[0x35] = new ItemChest(0x35, ITEM_EKEEKE, "Thieves Hideout: right chest in double ekeeke chest room");
	_chests[0x36] = new ItemChest(0x36, ITEM_20_GOLDS, "Thieves Hideout: left chest in room before boss");
	_chests[0x37] = new ItemChest(0x37, ITEM_GOLDEN_STATUE, "Thieves Hideout: right chest in room before boss");
	_chests[0x38] = new ItemChest(0x38, ITEM_LITHOGRAPH, "Thieves Hideout: lithograph chest in boss reward room");
	_chests[0x39] = new ItemChest(0x39, ITEM_5_GOLDS, "Thieves Hideout: 5 golds chest in boss reward room");
	_chests[0x3A] = new ItemChest(0x3A, ITEM_50_GOLDS, "Thieves Hideout: 50 golds chest in boss reward room");
	_chests[0x3B] = new ItemChest(0x3B, ITEM_LIFESTOCK, "Thieves Hideout: lifestock chest in boss reward room");
	_chests[0x3C] = new ItemChest(0x3C, ITEM_EKEEKE, "Thieves Hideout: ekeeke chest in boss reward room");
	_chests[0x42] = new ItemChest(0x42, ITEM_LIFESTOCK, "Verla Mines: right chest in \"crate on spike\" room near entrance");
	_chests[0x43] = new ItemChest(0x43, ITEM_EKEEKE, "Verla Mines: left chest in \"crate on spike\" room near entrance");
	_chests[0x44] = new ItemChest(0x44, ITEM_LIFESTOCK, "Verla Mines: chest on isolated cliff in \"jar staircase\" room");
	_chests[0x45] = new ItemChest(0x45, ITEM_KEY, "Verla Mines: Dex reward chest");
	_chests[0x46] = new ItemChest(0x46, ITEM_KEY, "Verla Mines: Slasher reward chest");
	_chests[0x47] = new ItemChest(0x47, ITEM_GOLDEN_STATUE, "Verla Mines: left chest in 3 chests room");
	_chests[0x48] = new ItemChest(0x48, ITEM_EKEEKE, "Verla Mines: middle chest in 3 chests room");
	_chests[0x49] = new ItemChest(0x49, ITEM_EKEEKE, "Verla Mines: right chest in 3 chests room");
	_chests[0x4A] = new ItemChest(0x4A, ITEM_EKEEKE, "Verla Mines: right chest in button room near elevator shaft leading to Marley");
	_chests[0x4B] = new ItemChest(0x4B, ITEM_EKEEKE, "Verla Mines: left chest in button room near elevator shaft leading to Marley");
	_chests[0x4C] = new ItemChest(0x4C, ITEM_LIFESTOCK, "Verla Mines: chest in hidden room accessible by lava-walking");
	_chests[0x4D] = new ItemChest(0x4D, ITEM_KEY, "Destel Well (0F): \"crates and holes\" room key chest");
	_chests[0x4E] = new ItemChest(0x4E, ITEM_EKEEKE, "Destel Well (1F): ekeeke chest on small stairs");
	_chests[0x4F] = new ItemChest(0x4F, ITEM_LIFESTOCK, "Destel Well (1F): lifestock chest on narrow ground");
	_chests[0x50] = new ItemChest(0x50, ITEM_LIFESTOCK, "Destel Well (1F): lifestock chest in spike room");
	_chests[0x51] = new ItemChest(0x51, ITEM_LIFESTOCK, "Destel Well (2F): lifestock chest");
	_chests[0x52] = new ItemChest(0x52, ITEM_DAHL, "Destel Well (2F): dahl chest");
	_chests[0x53] = new ItemChest(0x53, ITEM_LIFESTOCK, "Destel Well (2F): right chest in Pockets room");
	_chests[0x54] = new ItemChest(0x54, ITEM_GAIA_STATUE, "Destel Well (2F): left chest in Pockets room");
	_chests[0x55] = new ItemChest(0x55, ITEM_KEY, "Destel Well (3F): key chest in first trigger room");
	_chests[0x56] = new ItemChest(0x56, ITEM_KEY, "Destel Well (3F): key chest in giants room");
	_chests[0x57] = new ItemChest(0x57, ITEM_KEY, "Destel Well (3F): key chest in second trigger room");
	_chests[0x58] = new ItemChest(0x58, ITEM_MIND_REPAIR, "Destel Well (4F): top chest in room before Quake");
	_chests[0x59] = new ItemChest(0x59, ITEM_DAHL, "Destel Well (4F): left chest in room before Quake");
	_chests[0x5A] = new ItemChest(0x5A, ITEM_EKEEKE, "Destel Well (4F): down chest in room before Quake");
	_chests[0x5B] = new ItemChest(0x5B, ITEM_EKEEKE, "Destel Well (4F): right chest in room before Quake");
	_chests[0x5C] = new ItemChest(0x5C, ITEM_KEY, "Lake Shrine (-1F): green golem spinner key chest");
	_chests[0x5D] = new ItemChest(0x5D, ITEM_GOLDEN_STATUE, "Lake Shrine (-1F): golden statue chest in corridor");
	_chests[0x5E] = new ItemChest(0x5E, ITEM_LIFESTOCK, "Lake Shrine (-1F): green golem spinner lifestock chest");
	_chests[0x5F] = new ItemChest(0x5F, ITEM_LIFESTOCK, "Lake Shrine (-1F): golem hopping lifestock chest");
	_chests[0x60] = new ItemChest(0x60, ITEM_LIFESTOCK, "Lake Shrine (-2F): middle life stock");
	_chests[0x61] = new ItemChest(0x61, ITEM_LIFESTOCK, "Lake Shrine (-2F): \"throne room\" lifestock chest");
	_chests[0x62] = new ItemChest(0x62, ITEM_KEY, "Lake Shrine (-2F): \"throne room\" key chest");
	_chests[0x63] = new ItemChest(0x63, ITEM_LIFESTOCK, "Lake Shrine (-3F): white golems room");
	_chests[0x64] = new ItemChest(0x64, ITEM_KEY, "Lake Shrine (-3F): key chest near sword of ice");
	_chests[0x65] = new ItemChest(0x65, ITEM_LIFESTOCK, "Lake Shrine (-3F): chest in snake caging room");
	_chests[0x66] = new ItemChest(0x66, ITEM_LIFESTOCK, "Lake Shrine (-3F): lifestock chest on central block, obtained by falling from above");
	_chests[0x67] = new ItemChest(0x67, ITEM_DAHL, "Lake Shrine (-3F): chest before reaching the duke");
	_chests[0x68] = new ItemChest(0x68, ITEM_EKEEKE, "Lake Shrine (-3F): reward chest (left) after beating the duke");
	_chests[0x69] = new ItemChest(0x69, ITEM_LIFESTOCK, "Lake Shrine (-3F): reward chest (middle) after beating the duke");
	_chests[0x6A] = new ItemChest(0x6A, ITEM_EKEEKE, "Lake Shrine (-3F): reward chest (right) after beating the duke");
	_chests[0x6B] = new ItemChest(0x6B, ITEM_KEY, "Lake Shrine (-3F): key chest near golden golem spinner");
	_chests[0x6C] = new ItemChest(0x6C, ITEM_KEY, "King Nole's Labyrinth (0F): key chest in \"outside room\"");
	_chests[0x6D] = new ItemChest(0x6D, ITEM_EKEEKE, "King Nole's Labyrinth (0F): ekeeke chest in room after key door");
	_chests[0x6E] = new ItemChest(0x6E, ITEM_LIFESTOCK, "King Nole's Labyrinth (0F): lifestock chest in room after key door");
	_chests[0x6F] = new ItemChest(0x6F, ITEM_LIFESTOCK, "King Nole's Labyrinth (-1F): lifestock chest in \"small maze\" room");
	_chests[0x70] = new ItemChest(0x70, ITEM_GAIA_STATUE, "King Nole's Labyrinth (0F): chest in spike balls room");
	_chests[0x71] = new ItemChest(0x71, ITEM_EKEEKE, "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (left side)");
	_chests[0x72] = new ItemChest(0x72, ITEM_EKEEKE, "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (right side)");
	_chests[0x73] = new ItemChest(0x73, ITEM_RESTORATION, "King Nole's Labyrinth (-1F): restoration chest in triple chest dark room (left side)");
	_chests[0x74] = new ItemChest(0x74, ITEM_LIFESTOCK, "King Nole's Labyrinth (-1F): lifestock chest in \"big maze\" room");
	_chests[0x75] = new ItemChest(0x75, ITEM_EKEEKE, "King Nole's Labyrinth (-1F): ekeeke chest in lantern room");
	_chests[0x76] = new ItemChest(0x76, ITEM_LANTERN, "King Nole's Labyrinth (-1F): lantern chest");
	_chests[0x77] = new ItemChest(0x77, ITEM_KEY, "King Nole's Labyrinth (-1F): key chest in ice shortcut room");
	_chests[0x78] = new ItemChest(0x78, ITEM_EKEEKE, "King Nole's Labyrinth (-2F): ekeeke chest in skeleton priest room");
	_chests[0x79] = new ItemChest(0x79, ITEM_KEY, "King Nole's Labyrinth (-1F): key chest in \"button and crates\" room");
	_chests[0x7A] = new ItemChest(0x7A, ITEM_EKEEKE, "King Nole's Labyrinth (-3F): ekeeke chest before Firedemon");
	_chests[0x7B] = new ItemChest(0x7B, ITEM_DAHL, "King Nole's Labyrinth (-3F): dahl chest before Firedemon");
	_chests[0x7C] = new ItemChest(0x7C, ITEM_GOLA_NAIL, "King Nole's Labyrinth (-3F): reward for beating Firedemon");
	_chests[0x7D] = new ItemChest(0x7D, ITEM_LIFESTOCK, "King Nole's Labyrinth (-2F): lifestock chest in four buttons room");
	_chests[0x7E] = new ItemChest(0x7E, ITEM_EKEEKE, "King Nole's Labyrinth (-3F): first ekeeke chest before Spinner");
	_chests[0x7F] = new ItemChest(0x7F, ITEM_EKEEKE, "King Nole's Labyrinth (-3F): second ekeeke chest before Spinner");
	_chests[0x80] = new ItemChest(0x80, ITEM_GAIA_STATUE, "King Nole's Labyrinth (-3F): statue of gaia chest before Spinner");
	_chests[0x81] = new ItemChest(0x81, ITEM_GOLA_FANG, "King Nole's Labyrinth (-3F): reward for beating Spinner");
	_chests[0x82] = new ItemChest(0x82, ITEM_KEY, "King Nole's Labyrinth (-3F): key chest in Hyper Breast room");
	_chests[0x83] = new ItemChest(0x83, ITEM_GARLIC, "King Nole's Labyrinth (-3F): chest before Miro");
	_chests[0x84] = new ItemChest(0x84, ITEM_GOLA_HORN, "King Nole's Labyrinth (-3F): reward for beating Miro");
	_chests[0x85] = new ItemChest(0x85, ITEM_LIFESTOCK, "King Nole's Labyrinth (-3F): chest in hands room");
	_chests[0x86] = new ItemChest(0x86, ITEM_LIFESTOCK, "Route between Gumi and Ryuma: chest on the way to Swordsman Kado");
	_chests[0x87] = new ItemChest(0x87, ITEM_LIFESTOCK, "Route between Massan and Gumi: chest on promontory");
	_chests[0x88] = new ItemChest(0x88, ITEM_GOLDEN_STATUE, "Route between Mercator and Verla: golden statue chest on promontory");
	_chests[0x89] = new ItemChest(0x89, ITEM_RESTORATION, "Route between Mercator and Verla: restoration chest on promontory");
	_chests[0x8A] = new ItemChest(0x8A, ITEM_LIFESTOCK, "Route between Mercator and Verla: chest near Friday's village");
	_chests[0x8B] = new ItemChest(0x8B, ITEM_LIFESTOCK, "Verla Shore: chest on angle promontory after Verla tunnel");
	_chests[0x8C] = new ItemChest(0x8C, ITEM_LIFESTOCK, "Verla Shore: chest on highest promontory after Verla tunnel (accessible through Verla mines)");
	_chests[0x8D] = new ItemChest(0x8D, ITEM_LIFESTOCK, "Route to Mir Tower: chest on promontory accessed by pressing hidden switch");
	_chests[0x8E] = new ItemChest(0x8E, ITEM_DAHL, "Route to Mir Tower: dahl chest behind sacred tree");
	_chests[0x8F] = new ItemChest(0x8F, ITEM_LIFESTOCK, "Verla Shore: chest behind cabin");
	_chests[0x90] = new ItemChest(0x90, ITEM_DAHL, "Route to Destel: chest in map right after Verla mines exit");
	_chests[0x91] = new ItemChest(0x91, ITEM_LIFESTOCK, "Route to Destel: chest in \"elevator\" map");
	_chests[0x92] = new ItemChest(0x92, ITEM_LIFESTOCK, "Route to Mir Tower: lifestock chest behind sacred tree");
	_chests[0x93] = new ItemChest(0x93, ITEM_LIFESTOCK, "Route to Destel: hidden chest in map right before Destel");
	_chests[0x94] = new ItemChest(0x94, ITEM_DAHL, "Mountainous Area: chest near teleport tree");
	_chests[0x95] = new ItemChest(0x95, ITEM_LIFESTOCK, "Mountainous Area: chest on right side of the map right before the bridge");
	_chests[0x96] = new ItemChest(0x96, ITEM_GOLDEN_STATUE, "Mountainous Area: hidden chest in narrow path");
	_chests[0x98] = new ItemChest(0x98, ITEM_LIFESTOCK, "Mountainous Area: isolated life stock in bridge map");
	_chests[0x99] = new ItemChest(0x99, ITEM_EKEEKE, "Mountainous Area: left chest on wall in bridge map");
	_chests[0x9A] = new ItemChest(0x9A, ITEM_GOLDEN_STATUE, "Mountainous Area: right chest on wall in bridge map");
	_chests[0x9B] = new ItemChest(0x9B, ITEM_RESTORATION, "Mountainous Area: restoration chest in map before Zak arena");
	_chests[0x9C] = new ItemChest(0x9C, ITEM_EKEEKE, "Mountainous Area: ekeeke chest in map before Zak arena");
	_chests[0x9D] = new ItemChest(0x9D, ITEM_LIFESTOCK, "Route after Destel: chest on cliff angle");
	_chests[0x9E] = new ItemChest(0x9E, ITEM_LIFESTOCK, "Route after Destel: lifestock chest in map after seeing Duke raft");
	_chests[0x9F] = new ItemChest(0x9F, ITEM_DAHL, "Route after Destel: dahl chest in map after seeing Duke raft");
	_chests[0xA0] = new ItemChest(0xA0, ITEM_LIFESTOCK, "Mountainous Area: chest hidden under rocky arch");
	_chests[0xA1] = new ItemChest(0xA1, ITEM_LIFESTOCK, "Route to Lake Shrine: \"easy\" chest on crossroads with mountainous area");
	_chests[0xA2] = new ItemChest(0xA2, ITEM_LIFESTOCK, "Route to Lake Shrine: \"hard\" chest on crossroads with mountainous area");
	_chests[0xA3] = new ItemChest(0xA3, ITEM_EKEEKE, "Mountainous Area: chest in map in front of the statue under the bridge");
	_chests[0xA4] = new ItemChest(0xA4, ITEM_GAIA_STATUE, "Route to Lake Shrine: right chest in volcano");
	_chests[0xA5] = new ItemChest(0xA5, ITEM_GAIA_STATUE, "Route to Lake Shrine: left chest in volcano");
	_chests[0xA6] = new ItemChest(0xA6, ITEM_LIFESTOCK, "Mountainous Area Cave: chest in small hidden room");
	_chests[0xA7] = new ItemChest(0xA7, ITEM_DAHL, "Mountainous Area Cave: chest in small visible room");
	_chests[0xA8] = new ItemChest(0xA8, ITEM_LIFESTOCK, "Greenmaze: chest on path to lumberjack");
	_chests[0xA9] = new ItemChest(0xA9, ITEM_20_GOLDS, "Greenmaze: chest on promontory appearing after pressing a button in other section");
	_chests[0xAA] = new ItemChest(0xAA, ITEM_DAHL, "Greenmaze: chest between Sunstone and Massan shortcut");
	_chests[0xAB] = new ItemChest(0xAB, ITEM_LIFESTOCK, "Greenmaze: chest in mages room");
	_chests[0xAC] = new ItemChest(0xAC, ITEM_LIFESTOCK, "Greenmaze: left chest in elbow cave");
	_chests[0xAD] = new ItemChest(0xAD, ITEM_DAHL, "Greenmaze: right chest in elbow cave");
	_chests[0xAE] = new ItemChest(0xAE, ITEM_DAHL, "Greenmaze: chest in waterfall cave");
	_chests[0xAF] = new ItemChest(0xAF, ITEM_LIFESTOCK, "Greenmaze: left chest in hidden room behind waterfall ");
	_chests[0xB0] = new ItemChest(0xB0, ITEM_20_GOLDS, "Greenmaze: right chest in hidden room behind waterfall ");
	_chests[0xB1] = new ItemChest(0xB1, ITEM_LIFESTOCK, "Massan: chest triggered by dog statue");
	_chests[0xB2] = new ItemChest(0xB2, ITEM_EKEEKE, "Massan: chest in house nearest to elder house");
	_chests[0xB3] = new ItemChest(0xB3, ITEM_LIFESTOCK, "Massan: lifestock chest in house");
	_chests[0xB4] = new ItemChest(0xB4, ITEM_EKEEKE, "Massan: chest in house farthest from elder house");
	_chests[0xB5] = new ItemChest(0xB5, ITEM_LIFESTOCK, "Gumi: chest on top of bed in house");
	_chests[0xB6] = new ItemChest(0xB6, ITEM_LIFESTOCK, "Gumi: chest in elder house after saving Fara");
	_chests[0xB7] = new ItemChest(0xB7, ITEM_LIFESTOCK, "Ryuma: chest in mayor's house");
	_chests[0xB8] = new ItemChest(0xB8, ITEM_LIFESTOCK, "Ryuma: chest in repaired lighthouse");
	_chests[0xB9] = new ItemChest(0xB9, ITEM_LIFESTOCK, "Crypt: chest in main room");
	_chests[0xBA] = new ItemChest(0xBA, ITEM_ARMLET, "Crypt: reward chest");
	_chests[0xBF] = new ItemChest(0xBF, ITEM_DAHL, "Mercator: hidden casino chest");
	_chests[0xC0] = new ItemChest(0xC0, ITEM_LIFESTOCK, "Mercator: chest in Greenpea's house");
	_chests[0xC1] = new ItemChest(0xC1, ITEM_LIFESTOCK, "Mercator: chest in grandma's house (pot shelving trial)");
	_chests[0xC2] = new ItemChest(0xC2, ITEM_THUNDER_SWORD, "Verla: after beating Marley");
	_chests[0xC4] = new ItemChest(0xC4, ITEM_LIFESTOCK, "Destel: chest in shop requiring to wait for the shopkeeper to move");
	_chests[0xC5] = new ItemChest(0xC5, ITEM_GARLIC, "Mir Tower: garlic chest");
	_chests[0xC6] = new ItemChest(0xC6, ITEM_EKEEKE, "Mir Tower: chest after mimic room");
	_chests[0xC7] = new ItemChest(0xC7, ITEM_NONE, "Mir Tower: mimic room empty chest 1");
	_chests[0xC8] = new ItemChest(0xC8, ITEM_NONE, "Mir Tower: mimic room empty chest 2");
	_chests[0xC9] = new ItemChest(0xC9, ITEM_NONE, "Mir Tower: mimic room empty chest 3");
	_chests[0xCA] = new ItemChest(0xCA, ITEM_NONE, "Mir Tower: mimic room empty chest 4");
	_chests[0xCB] = new ItemChest(0xCB, ITEM_EKEEKE, "Mir Tower: chest in mushroom pit room");
	_chests[0xCC] = new ItemChest(0xCC, ITEM_GAIA_STATUE, "Mir Tower: chest in room next to mummy switch room");
	_chests[0xCD] = new ItemChest(0xCD, ITEM_LIFESTOCK, "Mir Tower: chest in library accessible from teleporter maze");
	_chests[0xCE] = new ItemChest(0xCE, ITEM_EKEEKE, "Mir Tower: hidden chest in room before library");
	_chests[0xCF] = new ItemChest(0xCF, ITEM_LIFESTOCK, "Mir Tower: chest in falling spikeballs room");
	_chests[0xD0] = new ItemChest(0xD0, ITEM_KEY, "Mir Tower: chest in timed challenge room");
	_chests[0xD1] = new ItemChest(0xD1, ITEM_EKEEKE, "Mir Tower: chest in room where Miro closes the door");
	_chests[0xD2] = new ItemChest(0xD2, ITEM_EKEEKE, "Mir Tower: chest after room where Miro closes the door");
	_chests[0xD3] = new ItemChest(0xD3, ITEM_PURPLE_JEWEL,  "Mir Tower: reward chest");
	_chests[0xD4] = new ItemChest(0xD4, ITEM_EKEEKE, "Mir Tower: right chest in reward room");
	_chests[0xD5] = new ItemChest(0xD5, ITEM_EKEEKE, "Mir Tower: left chest in reward room");
	_chests[0xD6] = new ItemChest(0xD6, ITEM_LIFESTOCK, "Mir Tower: chest behind wall accessible after beating Mir");
	_chests[0xD7] = new ItemChest(0xD7, ITEM_LIFESTOCK, "Witch Helga's Hut: lifestock chest");
	_chests[0xD8] = new ItemChest(0xD8, ITEM_LIFESTOCK, "Massan Cave: lifestock chest");
	_chests[0xD9] = new ItemChest(0xD9, ITEM_DAHL, "Massan Cave: dahl chest");
	_chests[0xDA] = new ItemChest(0xDA, ITEM_LIFESTOCK, "Tibor: reward chest after boss");
	_chests[0xDB] = new ItemChest(0xDB, ITEM_20_GOLDS, "Tibor: chest in spike balls room");
	_chests[0xDC] = new ItemChest(0xDC, ITEM_GAIA_STATUE, "Tibor: left chest on 2 chest group");
	_chests[0xDD] = new ItemChest(0xDD, ITEM_20_GOLDS, "Tibor: right chest on 2 chest group");

	// The following chests are absent from the game on release, and their IDs are therefore free:
	// 0x0E, 0x17, 0x1A, 0x1B, 0x1E, 0x20, 0x25, 0x27, 0x28, 0x33, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x97, 0xBB, 0xBC, 0xBD, 0xBE, 0xC3

	_pedestals[ItemPedestalCode::IDOL_STONE] = new ItemPedestal(0x021167, "Gumi: Idol Stone pedestal");
	_pedestals[ItemPedestalCode::SUN_STONE] = new ItemPedestal(0x020C21, "Greenmaze: Sun Stone pedestal");
	_pedestals[ItemPedestalCode::CHROME_BREAST] = new ItemPedestal(0x01DDB7, "Verla Mines: Chrome Breast pedestal");
	_pedestals[ItemPedestalCode::SHELL_BREAST] = new ItemPedestal(0x01EC99, "Lake Shrine (-3F): Shell Breast pedestal");
	_pedestals[ItemPedestalCode::HYPER_BREAST] = new ItemPedestal(0x01F9BD, "King Nole's Labyrinth (-3F): Hyper Breast pedestal");
	_pedestals[ItemPedestalCode::HEALING_BOOTS] = new ItemPedestal(0x01E247, "Destell Well: Healing Boots pedestal");
	_pedestals[ItemPedestalCode::IRON_BOOTS] = new ItemPedestal(0x01F36F, "King Nole's Labyrinth (-1F): Iron Boots pedestal");
	_pedestals[ItemPedestalCode::FIREPROOF] = new ItemPedestal(0x022C23, "Massan Cave: Fireproof Boots pedestal");
	_pedestals[ItemPedestalCode::SPIKE_BOOTS] = new ItemPedestal(0x01FAC1, "King Nole's Labyrinth (-3F): Snow Spikes pedestal");
	_pedestals[ItemPedestalCode::GAIA_SWORD] = new ItemPedestal(0x01F183, "King Nole's Labyrinth (-2F): Sword of Gaia pedestal");
	_pedestals[ItemPedestalCode::MARS_STONE] = new ItemPedestal(0x020419, "Route after Destel: Mars Stone pedestal");
	_pedestals[ItemPedestalCode::MOON_STONE] = new ItemPedestal(0x020AED, "Mountainous Area cave: Moon Stone pedestal");
	_pedestals[ItemPedestalCode::SATURN_STONE] = new ItemPedestal(0x0203AB, "Witch Helga's Hut: Saturn Stone pedestal");
	_pedestals[ItemPedestalCode::VENUS_STONE] = new ItemPedestal(0x01F8D7, "King Nole's Labyrinth (-3F): Venus Stone pedestal");
	_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_1] = new ItemPedestal(0x01E873, "Lake Shrine (-2F): north EkeEke pedestal");
	_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_2] = new ItemPedestal(0x01E87B, "Lake Shrine (-2F): south EkeEke pedestal");
	_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_3] = new ItemPedestal(0x01E883, "Lake Shrine (-2F): west EkeEke pedestal");
	_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_4] = new ItemPedestal(0x01E88B, "Lake Shrine (-2F): east EkeEke pedestal");
	_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_1] = new ItemPedestal(0x02011D, "Friday's Village: first EkeEke pedestal");
	_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_2] = new ItemPedestal(0x020115, "Friday's Village: second EkeEke pedestal");
	_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_3] = new ItemPedestal(0x02010D, "Friday's Village: third EkeEke pedestal");
//	_pedestals[ItemPedestalCode::MIR_TOWER_EKEEKE] = new ItemPedestal(0x02269C, "Mir Tower: EkeEke pedestal in priest room");
//	_pedestals[ItemPedestalCode::MIR_TOWER_DETOX] = new ItemPedestal(0x0226A3, "Mir Tower: Detox Grass pedestal in priest room");
	_pedestals[ItemPedestalCode::LOGS_1] = new ItemPedestal(0x01FA43, "King Nole's Labyrinth (-2F): left Logs pedestal");
	_pedestals[ItemPedestalCode::LOGS_2] = new ItemPedestal(0x01FA3B, "King Nole's Labyrinth (-2F): right Logs pedestal");

	// Ice sword pedestal is special in the sense that it's a double one (one that can be seen before pressing the button, and another one in the map with the bridge)
	_pedestals[ItemPedestalCode::ICE_SWORD] = new ItemPedestal(0x01EE37, "Lake Shrine (-3F): Sword of Ice pedestal");
	_pedestals[ItemPedestalCode::ICE_SWORD]->addOtherAddress(0x01EE41);

	_pedestals[ItemPedestalCode::MASSAN_SHOP_LIFESTOCK] = new ItemPedestal(0x02101D, "Massan shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::MASSAN_SHOP_EKEEKE_1] = new ItemPedestal(0x021015, "Massan shop: first EkeEke slot", true);
	_pedestals[ItemPedestalCode::MASSAN_SHOP_EKEEKE_2] = new ItemPedestal(0x02100D, "Massan shop: second EkeEke slot", true);
	_pedestals[ItemPedestalCode::GUMI_SHOP_MAP] = new ItemPedestal(0x0211DD, "Gumi shop: Map slot", true);
	_pedestals[ItemPedestalCode::GUMI_SHOP_LIFESTOCK] = new ItemPedestal(0x0211E5, "Gumi shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::GUMI_SHOP_EKEEKE] = new ItemPedestal(0x0211D5, "Gumi shop: Map slot", true);
	_pedestals[ItemPedestalCode::RYUMA_SHOP_LIFESTOCK] = new ItemPedestal(0x0212D9, "Ryuma shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::RYUMA_SHOP_GAIA_STATUE] = new ItemPedestal(0x0212C9, "Ryuma shop: Statue of Gaia slot", true);
	_pedestals[ItemPedestalCode::RYUMA_SHOP_GOLDEN_STATUE] = new ItemPedestal(0x0212B9, "Ryuma shop: Golden Statue slot", true);
	_pedestals[ItemPedestalCode::RYUMA_SHOP_EKEEKE] = new ItemPedestal(0x0212D1, "Ryuma shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::RYUMA_SHOP_DETOX_GRASS] = new ItemPedestal(0x0212C1, "Ryuma shop: Detox Grass slot", true);
	_pedestals[ItemPedestalCode::RYUMA_INN_EKEEKE] = new ItemPedestal(0x02139F, "Ryuma inn: EkeEke slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_ARMOR] = new ItemPedestal(0x021B7B, "Mercator shop: Steel Breast slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_BELL] = new ItemPedestal(0x021B83, "Mercator shop: Bell slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_EKEEKE] = new ItemPedestal(0x021B73, "Mercator shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_DETOX_GRASS] = new ItemPedestal(0x021B6B, "Mercator shop: Detox Grass slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_GAIA_STATUE] = new ItemPedestal(0x021B63, "Mercator shop: Statue of Gaia slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP_GOLDEN_STATUE] = new ItemPedestal(0x021B5B, "Mercator shop: Golden Statue slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP2_MIND_REPAIR] = new ItemPedestal(0x021CDF, "Mercator special shop: Mind Repair slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP2_ANTIPARALYZE] = new ItemPedestal(0x021CD7, "Mercator special shop: Anti Paralyze slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP2_DAHL] = new ItemPedestal(0x021CCF, "Mercator special shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::MERCATOR_SHOP2_RESTORATION] = new ItemPedestal(0x021CC7, "Mercator special shop: Restoration slot", true);
	_pedestals[ItemPedestalCode::VERLA_SHOP_LIFESTOCK] = new ItemPedestal(0x021F57, "Verla shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::VERLA_SHOP_EKEEKE] = new ItemPedestal(0x021F37, "Verla shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::VERLA_SHOP_DETOX_GRASS] = new ItemPedestal(0x021F3F, "Verla shop: Detox Grass slot", true);
	_pedestals[ItemPedestalCode::VERLA_SHOP_DAHL] = new ItemPedestal(0x021F47, "Verla shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::VERLA_SHOP_MAP] = new ItemPedestal(0x021F4F, "Verla shop: Map slot", true);
	_pedestals[ItemPedestalCode::KELKETO_SHOP_LIFESTOCK] = new ItemPedestal(0x020861, "Kelketo Waterfalls shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::KELKETO_SHOP_EKEEKE] = new ItemPedestal(0x020869, "Kelketo Waterfalls shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::KELKETO_SHOP_DETOX_GRASS] = new ItemPedestal(0x020871, "Kelketo Waterfalls shop: Detox Grass slot", true);
	_pedestals[ItemPedestalCode::KELKETO_SHOP_DAHL] = new ItemPedestal(0x020879, "Kelketo Waterfalls shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::KELKETO_SHOP_RESTORATION] = new ItemPedestal(0x020881, "Kelketo Waterfalls shop: Restoration slot", true);
	_pedestals[ItemPedestalCode::DESTEL_INN_EKEEKE] = new ItemPedestal(0x022017, "Destel inn: EkeEke slot", true);
	_pedestals[ItemPedestalCode::DESTEL_INN_MAP] = new ItemPedestal(0x022027, "Destel inn: Map slot", true);
	_pedestals[ItemPedestalCode::DESTEL_SHOP_EKEEKE] = new ItemPedestal(0x022055, "Destel shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::DESTEL_SHOP_DETOX_GRASS] = new ItemPedestal(0x02206D, "Destel shop: Detox Grass slot", true);
	_pedestals[ItemPedestalCode::DESTEL_SHOP_RESTORATION] = new ItemPedestal(0x022065, "Destel shop: Restoration slot", true);
	_pedestals[ItemPedestalCode::DESTEL_SHOP_DAHL] = new ItemPedestal(0x02205D, "Destel shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::DESTEL_SHOP_LIFE_STOCK] = new ItemPedestal(0x022075, "Destel shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::GREEDLY_SHOP_GAIA_STATUE] = new ItemPedestal(0x0209C7, "Greedly's shop: Statue of Gaia slot", true);
	_pedestals[ItemPedestalCode::GREEDLY_SHOP_GOLDEN_STATUE] = new ItemPedestal(0x0209BF, "Greedly's shop: Golden Statue slot", true);
	_pedestals[ItemPedestalCode::GREEDLY_SHOP_DAHL] = new ItemPedestal(0x0209CF, "Greedly's shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::GREEDLY_SHOP_LIFE_STOCK] = new ItemPedestal(0x0209AF, "Greedly's shop: Life Stock slot", true, true);
	_pedestals[ItemPedestalCode::KAZALT_SHOP_EKEEKE] = new ItemPedestal(0x022115, "Kazalt shop: EkeEke slot", true);
	_pedestals[ItemPedestalCode::KAZALT_SHOP_DAHL] = new ItemPedestal(0x022105, "Kazalt shop: Dahl slot", true);
	_pedestals[ItemPedestalCode::KAZALT_SHOP_GOLDEN_STATUE] = new ItemPedestal(0x02211D, "Kazalt shop: Golden Statue slot", true);
	_pedestals[ItemPedestalCode::KAZALT_SHOP_RESTORATION] = new ItemPedestal(0x02210D, "Kazalt shop: Restoration slot", true);

	_rewards[ItemRewardCode::MIR_AXE_MAGIC] = new ItemReward(0x028A3F, "Mir reward after Lake Shrine (Axe Magic in OG)");
	_rewards[ItemRewardCode::ZAK_GOLA_EYE] = new ItemReward(0x028A73, "Zak reward after fighting (Gola's Eye in OG)");
	_rewards[ItemRewardCode::SWORDSMAN_KADO] = new ItemReward(0x02894B, "Swordman Kado reward (Magic Sword in OG)");
	// _rewards[ItemRewardCode::RYUMA_MAYOR] = new ItemReward(0x02837B, "Ryuma's Mayor reward (Safety Pass in OG)");
	// _rewards[ItemRewardCode::LUMBERJACK] = new ItemReward(0x000000, "Greenmaze lumberjack reward (Einstein Whistle in OG)");
}

void World::initRegions()
{
	WorldRegion* massan = new WorldRegion("Massan");
	massan->addItemSource(_chests[0xB1]);     // "Massan: chest triggered by dog statue"
	massan->addItemSource(_chests[0xB2]);     // "Massan: chest in house nearest to elder house");
	massan->addItemSource(_chests[0xB3]);     // "Massan: lifestock chest in house");
	massan->addItemSource(_chests[0xB4]);     // "Massan: chest in house farthest from elder house");
	massan->addItemSource(_pedestals[ItemPedestalCode::MASSAN_SHOP_LIFESTOCK]);
	massan->addItemSource(_pedestals[ItemPedestalCode::MASSAN_SHOP_EKEEKE_1]);
	massan->addItemSource(_pedestals[ItemPedestalCode::MASSAN_SHOP_EKEEKE_2]);
	_regions.push_back(massan);
	_spawnRegion = massan;

	WorldRegion* massanCave = new WorldRegion("Massan Cave");
	massanCave->addItemSource(_chests[0xD8]); // "Massan Cave: lifestock chest");
	massanCave->addItemSource(_chests[0xD9]); // "Massan Cave: dahl chest");
	massanCave->addItemSource(_pedestals[ItemPedestalCode::FIREPROOF]);
	_regions.push_back(massanCave);

	WorldRegion* routeMassanToGumi = new WorldRegion("Route from Massan to Gumi");
	routeMassanToGumi->addItemSource(_chests[0x87]);  // "Route between Massan and Gumi: chest on promontory");
	_regions.push_back(routeMassanToGumi);

	WorldRegion* waterfallShrine = new WorldRegion("Waterfall Shrine");
	waterfallShrine->addItemSource(_chests[0x21]); // "Waterfall Shrine: lifestock chest under entrance (accessible after talking with Prospero)");
	waterfallShrine->addItemSource(_chests[0x22]); // "Waterfall Shrine: lifestock chest near Prospero");
	waterfallShrine->addItemSource(_chests[0x23]); // "Waterfall Shrine: chest in button room");
	waterfallShrine->addItemSource(_chests[0x24]); // "Waterfall Shrine: upstairs key chest");
	_regions.push_back(waterfallShrine);

	WorldRegion* swampShrine = new WorldRegion("Swamp Shrine");
	swampShrine->addItemSource(_chests[0x00]);    // "Swamp Shrine (0F): ekeeke chest in room to the right, close to door");
	swampShrine->addItemSource(_chests[0x01]);    // "Swamp Shrine (0F): key chest in carpet room");
	swampShrine->addItemSource(_chests[0x02]);    // "Swamp Shrine (0F): lifestock chest in room to the left, accessible by falling from upstairs");
	swampShrine->addItemSource(_chests[0x03]);    // "Swamp Shrine (0F): ekeeke chest falling from the ceiling when beating the orc");
	swampShrine->addItemSource(_chests[0x04]);    // "Swamp Shrine (0F): ekeeke chest in room connected to second entrance (without idol stone)");
	swampShrine->addItemSource(_chests[0x05]);    // "Swamp Shrine (1F): lower key chest in wooden bridges room");
	swampShrine->addItemSource(_chests[0x06]);    // "Swamp Shrine (2F): upper key chest in wooden bridges room");
	swampShrine->addItemSource(_chests[0x07]);    // "Swamp Shrine (1F): key chest in spike room");
	swampShrine->addItemSource(_chests[0x08]);    // "Swamp Shrine (2F): lifestock chest in Fara's room");
	swampShrine->addItemSource(_chests[0xB6]);    // "Gumi: chest in elder house after saving Fara");
	_regions.push_back(swampShrine);

	WorldRegion* gumi = new WorldRegion("Gumi");
	gumi->addItemSource(_chests[0xB5]);       // "Gumi: chest on top of bed in house");
	gumi->addItemSource(_pedestals[ItemPedestalCode::IDOL_STONE]);
	gumi->addItemSource(_pedestals[ItemPedestalCode::GUMI_SHOP_MAP]);
	gumi->addItemSource(_pedestals[ItemPedestalCode::GUMI_SHOP_LIFESTOCK]);
	gumi->addItemSource(_pedestals[ItemPedestalCode::GUMI_SHOP_EKEEKE]);
	_regions.push_back(gumi);

	WorldRegion* routeGumiToRyuma = new WorldRegion("Route from Gumi to Ryuma");
	routeGumiToRyuma->addItemSource(_chests[0x86]);   // "Route between Gumi and Ryuma: chest on the way to Swordsman Kado"
	routeGumiToRyuma->addItemSource(_rewards[ItemRewardCode::SWORDSMAN_KADO]);
	_regions.push_back(routeGumiToRyuma);

	WorldRegion* tibor = new WorldRegion("Tibor");
	tibor->addItemSource(_chests[0xDA]);  // "Tibor: reward chest after boss"
	tibor->addItemSource(_chests[0xDB]);  // "Tibor: chest in spike balls room"
	tibor->addItemSource(_chests[0xDC]);  // "Tibor: left chest on 2 chest group"
	tibor->addItemSource(_chests[0xDD]);  // "Tibor: right chest on 2 chest group"
	_regions.push_back(tibor);

	WorldRegion* ryuma = new WorldRegion("Ryuma");
	ryuma->addItemSource(_chests[0xB7]);      // "Ryuma: chest in mayor's house");
	ryuma->addItemSource(_chests[0xB8]);      // "Ryuma: chest in repaired lighthouse");
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_SHOP_LIFESTOCK]);
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_SHOP_GAIA_STATUE]);
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_SHOP_GOLDEN_STATUE]);
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_SHOP_EKEEKE]);
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_SHOP_DETOX_GRASS]);
	ryuma->addItemSource(_pedestals[ItemPedestalCode::RYUMA_INN_EKEEKE]);
	_regions.push_back(ryuma);

	WorldRegion* thievesHideout = new WorldRegion("Thieves Hideout");
	thievesHideout->addItemSource(_chests[0x26]); // "Thieves Hideout: chest in entrance room when water is removed");
	thievesHideout->addItemSource(_chests[0x29]); // "Thieves Hideout: right chest in room accessible by falling from miniboss room");
	thievesHideout->addItemSource(_chests[0x2A]); // "Thieves Hideout: left chest in room accessible by falling from miniboss room");
	thievesHideout->addItemSource(_chests[0x2B]); // "Thieves Hideout: left chest in Pockets cell");
	thievesHideout->addItemSource(_chests[0x2C]); // "Thieves Hideout: right chest in Pockets cell");
	thievesHideout->addItemSource(_chests[0x2D]); // "Thieves Hideout: right chest in room after quick climb trial");
	thievesHideout->addItemSource(_chests[0x2E]); // "Thieves Hideout: left chest in room after quick climb trial");
	thievesHideout->addItemSource(_chests[0x2F]); // "Thieves Hideout: chest in first platform room");
	thievesHideout->addItemSource(_chests[0x30]); // "Thieves Hideout: chest in second platform room");
	thievesHideout->addItemSource(_chests[0x31]); // "Thieves Hideout: reward chest after moving balls room");
	thievesHideout->addItemSource(_chests[0x32]); // "Thieves Hideout: chest near rolling boulder");
	thievesHideout->addItemSource(_chests[0x34]); // "Thieves Hideout: left chest in double ekeeke chest room");
	thievesHideout->addItemSource(_chests[0x35]); // "Thieves Hideout: right chest in double ekeeke chest room");
	thievesHideout->addItemSource(_chests[0x36]); // "Thieves Hideout: left chest in room before boss");
	thievesHideout->addItemSource(_chests[0x37]); // "Thieves Hideout: right chest in room before boss");
	thievesHideout->addItemSource(_chests[0x38]); // "Thieves Hideout: lithograph chest in boss reward room");
	thievesHideout->addItemSource(_chests[0x39]); // "Thieves Hideout: 5 golds chest in boss reward room");
	thievesHideout->addItemSource(_chests[0x3A]); // "Thieves Hideout: 50 golds chest in boss reward room");
	thievesHideout->addItemSource(_chests[0x3B]); // "Thieves Hideout: lifestock chest in boss reward room");
	thievesHideout->addItemSource(_chests[0x3C]); // "Thieves Hideout: ekeeke chest in boss reward room");
	// thievesHideout->addItemSource(_rewards[ItemRewardCode::RYUMA_MAYOR]);
	_regions.push_back(thievesHideout);

	WorldRegion* witchHelgaHut = new WorldRegion("Witch Helga's Hut");
	witchHelgaHut->addItemSource(_chests[0xD7]);  // "Witch Helga's Hut: lifestock chest");
	witchHelgaHut->addItemSource(_pedestals[ItemPedestalCode::SATURN_STONE]);
	_regions.push_back(witchHelgaHut);

	WorldRegion* mercator = new WorldRegion("Mercator");
	mercator->addItemSource(_chests[0x0D]);       // "Mercator: castle kitchen chest");
	mercator->addItemSource(_chests[0x11]);       // "Mercator: Arthur key chest in castle tower");
	mercator->addItemSource(_chests[0xC0]);       // "Mercator: chest in Greenpea's house");
	mercator->addItemSource(_chests[0xC1]);       // "Mercator: chest in grandma's house (pot shelving trial)");
	mercator->addItemSource(_chests[0xBF]);		  // "Mercator: hidden casino chest");
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_ARMOR]);
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_BELL]);
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_EKEEKE]);
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_DETOX_GRASS]);
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_GAIA_STATUE]);
	mercator->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP_GOLDEN_STATUE]);
	_regions.push_back(mercator);

	WorldRegion* mercatorSpecialShop = new WorldRegion("Mercator special shop");
	mercatorSpecialShop->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP2_MIND_REPAIR]);
	mercatorSpecialShop->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP2_ANTIPARALYZE]);
	mercatorSpecialShop->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP2_DAHL]);
	mercatorSpecialShop->addItemSource(_pedestals[ItemPedestalCode::MERCATOR_SHOP2_RESTORATION]);
	_regions.push_back(mercatorSpecialShop);

	WorldRegion* crypt = new WorldRegion("Crypt");
	crypt->addItemSource(_chests[0xB9]);          // "Crypt: chest in main room");
	crypt->addItemSource(_chests[0xBA]);          // "Crypt: reward chest");
	_regions.push_back(crypt);

	WorldRegion* mercatorDungeon = new WorldRegion("Mercator Dungeon");
	mercatorDungeon->addItemSource(_chests[0x09]);    // "Mercator Dungeon (-1F): lifestock chest after key door");
	mercatorDungeon->addItemSource(_chests[0x0A]);    // "Mercator Dungeon (-1F): key chest in Moralis's cell");
	mercatorDungeon->addItemSource(_chests[0x0B]);    // "Mercator Dungeon (-1F): left ekeeke chest in double chest room");
	mercatorDungeon->addItemSource(_chests[0x0C]);    // "Mercator Dungeon (-1F): right ekeeke chest in double chest room");
	mercatorDungeon->addItemSource(_chests[0x0F]);    // "Mercator Dungeon (1F): left ekeeke chest in double chest room");
	mercatorDungeon->addItemSource(_chests[0x10]);    // "Mercator Dungeon (1F): right ekeeke chest in double chest room");
	mercatorDungeon->addItemSource(_chests[0x12]);    // "Mercator Dungeon (4F): chest on top of tower");
	_regions.push_back(mercatorDungeon);

	WorldRegion* mirTowerSector = new WorldRegion("Mir Tower Sector");
	mirTowerSector->addItemSource(_chests[0x88]); // "Route between Mercator and Verla: golden statue chest on promontory");
	mirTowerSector->addItemSource(_chests[0x89]); // "Route between Mercator and Verla: restoration chest on promontory");
	mirTowerSector->addItemSource(_chests[0x8A]); // "Route between Mercator and Verla: chest near Friday's village");
	mirTowerSector->addItemSource(_chests[0x8D]); // "Route to Mir Tower: chest on promontory accessed by pressing hidden switch");
	mirTowerSector->addItemSource(_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_1]);
	mirTowerSector->addItemSource(_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_2]);
	mirTowerSector->addItemSource(_pedestals[ItemPedestalCode::FRIDAY_EKEEKE_3]);
	_regions.push_back(mirTowerSector);

	WorldRegion* mirTowerSectorSacredTrees = new WorldRegion("Mir Tower Sector (behind sacred trees)");
	mirTowerSectorSacredTrees->addItemSource(_chests[0x8E]);  // "Route to Mir Tower: dahl chest behind sacred tree");
	mirTowerSectorSacredTrees->addItemSource(_chests[0x92]);  // "Route to Mir Tower: lifestock chest behind sacred tree");
	_regions.push_back(mirTowerSectorSacredTrees);

	WorldRegion* mirTowerPreGarlic = new WorldRegion("Mir Tower (pre-garlic)");
	mirTowerPreGarlic->addItemSource(_chests[0xC5]);  // "Mir Tower: garlic chest");
	mirTowerPreGarlic->addItemSource(_chests[0xC6]);  // "Mir Tower: chest after mimic room");
	mirTowerPreGarlic->addItemSource(_chests[0xC7]);  // "Mir Tower: mimic room empty chest 1");
	mirTowerPreGarlic->addItemSource(_chests[0xC8]);  // "Mir Tower: mimic room empty chest 2");
	mirTowerPreGarlic->addItemSource(_chests[0xC9]);  // "Mir Tower: mimic room empty chest 3");
	mirTowerPreGarlic->addItemSource(_chests[0xCA]);  // "Mir Tower: mimic room empty chest 4");
	mirTowerPreGarlic->addItemSource(_chests[0xCB]);  // "Mir Tower: chest in mushroom pit room");
	_regions.push_back(mirTowerPreGarlic);

	WorldRegion* mirTowerPostGarlic = new WorldRegion("Mir Tower (post-garlic)");
	mirTowerPostGarlic->addItemSource(_chests[0xCC]);  // "Mir Tower: chest in room next to mummy switch room");
	mirTowerPostGarlic->addItemSource(_chests[0xCD]);  // "Mir Tower: chest in library accessible from teleporter maze");
	mirTowerPostGarlic->addItemSource(_chests[0xCE]);  // "Mir Tower: hidden chest in room before library");
	mirTowerPostGarlic->addItemSource(_chests[0xCF]);  // "Mir Tower: chest in falling spikeballs room");
	mirTowerPostGarlic->addItemSource(_chests[0xD0]);  // "Mir Tower: chest in timed challenge room");
	mirTowerPostGarlic->addItemSource(_chests[0xD1]);  // "Mir Tower: chest in room where Miro closes the door");
	mirTowerPostGarlic->addItemSource(_chests[0xD2]);  // "Mir Tower: chest after room where Miro closes the door");
	mirTowerPostGarlic->addItemSource(_chests[0xD3]);  // "Mir Tower: reward chest");
	mirTowerPostGarlic->addItemSource(_chests[0xD4]);  // "Mir Tower: right chest in reward room");
	mirTowerPostGarlic->addItemSource(_chests[0xD5]);  // "Mir Tower: left chest in reward room");
	mirTowerPostGarlic->addItemSource(_chests[0xD6]);  // "Mir Tower: chest behind wall accessible after beating Mir");
//	mirTowerPostGarlic->addItemSource(_pedestals[ItemPedestalCode::MIR_TOWER_EKEEKE]);
//	mirTowerPostGarlic->addItemSource(_pedestals[ItemPedestalCode::MIR_TOWER_DETOX]);
	_regions.push_back(mirTowerPostGarlic);

	WorldRegion* greenmaze = new WorldRegion("Greenmaze");
	greenmaze->addItemSource(_chests[0xA8]);   // "Greenmaze: chest on path to lumberjack");
	greenmaze->addItemSource(_chests[0xA9]);   // "Greenmaze: chest on promontory appearing after pressing a button in other section");
	greenmaze->addItemSource(_chests[0xAB]);   // "Greenmaze: chest in mages room");
	greenmaze->addItemSource(_chests[0xAC]);   // "Greenmaze: left chest in elbow cave");
	greenmaze->addItemSource(_chests[0xAD]);   // "Greenmaze: right chest in elbow cave");
	greenmaze->addItemSource(_chests[0xAE]);   // "Greenmaze: chest in waterfall cave");
	greenmaze->addItemSource(_chests[0xAF]);   // "Greenmaze: left chest in hidden room behind waterfall ");
	greenmaze->addItemSource(_chests[0xB0]);   // "Greenmaze: right chest in hidden room behind waterfall ");
	_regions.push_back(greenmaze);

	WorldRegion* greenmazeBehindSacredTrees = new WorldRegion("Greenmaze (behind sacred trees)");
	greenmazeBehindSacredTrees->addItemSource(_chests[0xAA]);   // "Greenmaze: chest between Sunstone and Massan shortcut");
	greenmazeBehindSacredTrees->addItemSource(_pedestals[ItemPedestalCode::SUN_STONE]);
	_regions.push_back(greenmazeBehindSacredTrees);

	WorldRegion* verlaShore = new WorldRegion("Verla Shore");
	verlaShore->addItemSource(_chests[0x8F]);     // "Verla Shore: chest behind cabin");
	verlaShore->addItemSource(_chests[0x8B]);     // "Verla Shore: chest on angle promontory after Verla tunnel");
	verlaShore->addItemSource(_chests[0x8C]);     // "Verla Shore: chest on highest promontory after Verla tunnel (accessible through Verla mines)");
	verlaShore->addItemSource(_pedestals[ItemPedestalCode::VERLA_SHOP_LIFESTOCK]);
	verlaShore->addItemSource(_pedestals[ItemPedestalCode::VERLA_SHOP_EKEEKE]);
	verlaShore->addItemSource(_pedestals[ItemPedestalCode::VERLA_SHOP_DETOX_GRASS]);
	verlaShore->addItemSource(_pedestals[ItemPedestalCode::VERLA_SHOP_DAHL]);
	verlaShore->addItemSource(_pedestals[ItemPedestalCode::VERLA_SHOP_MAP]);
	_regions.push_back(verlaShore);

	WorldRegion* verlaMines = new WorldRegion("Verla Mines");
	verlaMines->addItemSource(_chests[0x42]);     // "Verla Mines: right chest in \"crate on spike\" room near entrance");
	verlaMines->addItemSource(_chests[0x43]);     // "Verla Mines: left chest in \"crate on spike\" room near entrance");
	verlaMines->addItemSource(_chests[0x44]);     // "Verla Mines: chest on isolated cliff in \"jar staircase\" room");
	verlaMines->addItemSource(_chests[0x45]);     // "Verla Mines: Dex reward chest");
	verlaMines->addItemSource(_chests[0x46]);     // "Verla Mines: Slasher reward chest");
	verlaMines->addItemSource(_chests[0x47]);     // "Verla Mines: left chest in 3 chests room");
	verlaMines->addItemSource(_chests[0x48]);     // "Verla Mines: middle chest in 3 chests room");
	verlaMines->addItemSource(_chests[0x49]);     // "Verla Mines: right chest in 3 chests room");
	verlaMines->addItemSource(_chests[0x4A]);     // "Verla Mines: right chest in button room near elevator shaft leading to Marley");
	verlaMines->addItemSource(_chests[0x4B]);     // "Verla Mines: left chest in button room near elevator shaft leading to Marley");
	verlaMines->addItemSource(_chests[0x4C]);     // "Verla Mines: chest in hidden room accessible by lava-walking");
	verlaMines->addItemSource(_chests[0xC2]);     // "Verla: thunder sword chest after beating Marley"
	verlaMines->addItemSource(_pedestals[ItemPedestalCode::CHROME_BREAST]);
	_regions.push_back(verlaMines);

	WorldRegion* routeToDestel = new WorldRegion("Route to Destel");
	routeToDestel->addItemSource(_chests[0x90]);  // "Route to Destel: chest in map right after Verla mines exit");
	routeToDestel->addItemSource(_chests[0x91]);  // "Route to Destel: chest in \"elevator\" map");
	routeToDestel->addItemSource(_chests[0x93]);  // "Route to Destel: hidden chest in map right before Destel");
	routeToDestel->addItemSource(_pedestals[ItemPedestalCode::KELKETO_SHOP_LIFESTOCK]);
	routeToDestel->addItemSource(_pedestals[ItemPedestalCode::KELKETO_SHOP_EKEEKE]);
	routeToDestel->addItemSource(_pedestals[ItemPedestalCode::KELKETO_SHOP_DETOX_GRASS]);
	routeToDestel->addItemSource(_pedestals[ItemPedestalCode::KELKETO_SHOP_DAHL]);
	routeToDestel->addItemSource(_pedestals[ItemPedestalCode::KELKETO_SHOP_RESTORATION]);
	_regions.push_back(routeToDestel);

	WorldRegion* destel = new WorldRegion("Destel");
	destel->addItemSource(_chests[0xC4]);         // "Destel: chest in shop requiring to wait for the shopkeeper to move");
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_INN_EKEEKE]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_INN_MAP]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_SHOP_EKEEKE]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_SHOP_DETOX_GRASS]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_SHOP_RESTORATION]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_SHOP_DAHL]);
	destel->addItemSource(_pedestals[ItemPedestalCode::DESTEL_SHOP_LIFE_STOCK]);
	_regions.push_back(destel);

	WorldRegion* routeAfterDestel = new WorldRegion("Route after Destel");
	routeAfterDestel->addItemSource(_chests[0x9D]);     // "Route after Destel: chest on cliff angle");
	routeAfterDestel->addItemSource(_chests[0x9E]);     // "Route after Destel: lifestock chest in map after seeing Duke raft");
	routeAfterDestel->addItemSource(_chests[0x9F]);     // "Route after Destel: dahl chest in map after seeing Duke raft");
	routeAfterDestel->addItemSource(_pedestals[ItemPedestalCode::MARS_STONE]);
	_regions.push_back(routeAfterDestel);

	WorldRegion* destelWell = new WorldRegion("Destel Well");
	destelWell->addItemSource(_chests[0x4D]);     // "Destel Well (0F): \"crates and holes\" room key chest");
	destelWell->addItemSource(_chests[0x4E]);     // "Destel Well (1F): ekeeke chest on small stairs");
	destelWell->addItemSource(_chests[0x4F]);     // "Destel Well (1F): lifestock chest on narrow ground");
	destelWell->addItemSource(_chests[0x50]);     // "Destel Well (1F): lifestock chest in spike room");
	destelWell->addItemSource(_chests[0x51]);     // "Destel Well (2F): lifestock chest");
	destelWell->addItemSource(_chests[0x52]);     // "Destel Well (2F): dahl chest");
	destelWell->addItemSource(_chests[0x53]);     // "Destel Well (2F): right chest in Pockets room");
	destelWell->addItemSource(_chests[0x54]);     // "Destel Well (2F): left chest in Pockets room");
	destelWell->addItemSource(_chests[0x55]);     // "Destel Well (3F): key chest in first trigger room");
	destelWell->addItemSource(_chests[0x56]);     // "Destel Well (3F): key chest in giants room");
	destelWell->addItemSource(_chests[0x57]);     // "Destel Well (3F): key chest in second trigger room");
	destelWell->addItemSource(_chests[0x58]);     // "Destel Well (4F): top chest in room before Quake");
	destelWell->addItemSource(_chests[0x59]);     // "Destel Well (4F): left chest in room before Quake");
	destelWell->addItemSource(_chests[0x5A]);     // "Destel Well (4F): down chest in room before Quake");
	destelWell->addItemSource(_chests[0x5B]);     // "Destel Well (4F): right chest in room before Quake");
	destelWell->addItemSource(_pedestals[ItemPedestalCode::HEALING_BOOTS]);
	_regions.push_back(destelWell);

	WorldRegion* routeToLakeShrine = new WorldRegion("Route to Lake Shrine");
	routeToLakeShrine->addItemSource(_chests[0xA1]);  // "Route to Lake Shrine: \"easy\" chest on crossroads with mountainous area");
	routeToLakeShrine->addItemSource(_chests[0xA4]);  // "Route to Lake Shrine: right chest in volcano");
	routeToLakeShrine->addItemSource(_chests[0xA5]);  // "Route to Lake Shrine: left chest in volcano");
	routeToLakeShrine->addItemSource(_pedestals[ItemPedestalCode::GREEDLY_SHOP_GAIA_STATUE]);
	routeToLakeShrine->addItemSource(_pedestals[ItemPedestalCode::GREEDLY_SHOP_GOLDEN_STATUE]);
	routeToLakeShrine->addItemSource(_pedestals[ItemPedestalCode::GREEDLY_SHOP_DAHL]);
	routeToLakeShrine->addItemSource(_pedestals[ItemPedestalCode::GREEDLY_SHOP_LIFE_STOCK]);
	_regions.push_back(routeToLakeShrine);

	WorldRegion* lakeShrine = new WorldRegion("Lake Shrine");
	lakeShrine->addItemSource(_chests[0x5C]);     // "Lake Shrine (-1F): green golem spinner key chest");
	lakeShrine->addItemSource(_chests[0x5D]);     // "Lake Shrine (-1F): golden statue chest in corridor");
	lakeShrine->addItemSource(_chests[0x5E]);     // "Lake Shrine (-1F): green golem spinner lifestock chest");
	lakeShrine->addItemSource(_chests[0x5F]);     // "Lake Shrine (-1F): golem hopping lifestock chest");
	lakeShrine->addItemSource(_chests[0x60]);     // "Lake Shrine (-2F): middle life stock ");
	lakeShrine->addItemSource(_chests[0x61]);     // "Lake Shrine (-2F): \"throne room\" lifestock chest ");
	lakeShrine->addItemSource(_chests[0x62]);     // "Lake Shrine (-2F): \"throne room\" key chest ");
	lakeShrine->addItemSource(_chests[0x63]);     // "Lake Shrine (-3F): white golems room");
	lakeShrine->addItemSource(_chests[0x64]);     // "Lake Shrine (-3F): key chest near sword of ice ");
	lakeShrine->addItemSource(_chests[0x65]);     // "Lake Shrine (-3F): chest in snake caging room");
	lakeShrine->addItemSource(_chests[0x66]);     // "Lake Shrine (-3F): lifestock chest on central block, obtained by falling from above");
	lakeShrine->addItemSource(_chests[0x67]);     // "Lake Shrine (-3F): chest before reaching the duke");
	lakeShrine->addItemSource(_chests[0x68]);     // "Lake Shrine (-3F): reward chest (left) after beating the duke");
	lakeShrine->addItemSource(_chests[0x69]);     // "Lake Shrine (-3F): reward chest (middle) after beating the duke");
	lakeShrine->addItemSource(_chests[0x6A]);     // "Lake Shrine (-3F): reward chest (right) after beating the duke");
	lakeShrine->addItemSource(_chests[0x6B]);     // "Lake Shrine (-3F): key chest near golden golem spinner");
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::SHELL_BREAST]);
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::ICE_SWORD]);
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_1]);
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_2]);
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_3]);
	lakeShrine->addItemSource(_pedestals[ItemPedestalCode::LAKE_SHRINE_EKEEKE_4]);
	lakeShrine->addItemSource(_rewards[ItemRewardCode::MIR_AXE_MAGIC]);
	_regions.push_back(lakeShrine);

	WorldRegion* mountainousArea = new WorldRegion("Mountainous Area");
	mountainousArea->addItemSource(_chests[0x94]);   // "Mountainous Area: chest near teleport tree");
	mountainousArea->addItemSource(_chests[0x95]);   // "Mountainous Area: chest on right side of the map right before the bridge");
	mountainousArea->addItemSource(_chests[0x96]);   // "Mountainous Area: hidden chest in narrow path");
	mountainousArea->addItemSource(_chests[0x98]);   // "Mountainous Area: isolated life stock in bridge map");
	mountainousArea->addItemSource(_chests[0x99]);   // "Mountainous Area: left chest on wall in bridge map");
	mountainousArea->addItemSource(_chests[0x9A]);   // "Mountainous Area: right chest on wall in bridge map");
	mountainousArea->addItemSource(_chests[0x9B]);   // "Mountainous Area: restoration chest in map before Zak arena");
	mountainousArea->addItemSource(_chests[0x9C]);   // "Mountainous Area: ekeeke chest in map before Zak arena");
	mountainousArea->addItemSource(_chests[0xA0]);   // "Mountainous Area: chest hidden under rocky arch");
	mountainousArea->addItemSource(_chests[0xA3]);   // "Mountainous Area: chest in map in front of the statue under the bridge");
	mountainousArea->addItemSource(_chests[0xA6]);   // "Mountainous Area Cave: chest in small hidden room");
	mountainousArea->addItemSource(_chests[0xA7]);   // "Mountainous Area Cave: chest in small visible room");
	mountainousArea->addItemSource(_chests[0xA2]);   // "Route to Lake Shrine: \"hard\" chest on crossroads with mountainous area");
	mountainousArea->addItemSource(_pedestals[ItemPedestalCode::MOON_STONE]);
	mountainousArea->addItemSource(_rewards[ItemRewardCode::ZAK_GOLA_EYE]);
	_regions.push_back(mountainousArea);

	WorldRegion* kingNolesCave = new WorldRegion("King Nole's Cave");
	kingNolesCave->addItemSource(_chests[0x16]);  // "King Nole's Cave: first lifestock chest");
	kingNolesCave->addItemSource(_chests[0x18]);  // "King Nole's Cave: first gold chest in third room");
	kingNolesCave->addItemSource(_chests[0x19]);  // "King Nole's Cave: second gold chest in third room");
	kingNolesCave->addItemSource(_chests[0x1C]);  // "King Nole's Cave: gold chest in isolated room");
	kingNolesCave->addItemSource(_chests[0x1D]);  // "King Nole's Cave: lifestock chest in crate room");
	kingNolesCave->addItemSource(_chests[0x1F]);  // "King Nole's Cave: boulder chase corridor chest");
	_regions.push_back(kingNolesCave);

	WorldRegion* kazalt = new WorldRegion("Kazalt");
	kazalt->addItemSource(_pedestals[ItemPedestalCode::KAZALT_SHOP_EKEEKE]);
	kazalt->addItemSource(_pedestals[ItemPedestalCode::KAZALT_SHOP_DAHL]);
	kazalt->addItemSource(_pedestals[ItemPedestalCode::KAZALT_SHOP_GOLDEN_STATUE]);
	kazalt->addItemSource(_pedestals[ItemPedestalCode::KAZALT_SHOP_RESTORATION]);
	_regions.push_back(kazalt);

	WorldRegion* kingNolesLabyrinthEntrance = new WorldRegion("King Nole's Labyrinth (entrance)");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x6C]); // "King Nole's Labyrinth (0F): key chest in \"outside room\"");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x6D]); // "King Nole's Labyrinth (0F): ekeeke chest in room after key door");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x6E]); // "King Nole's Labyrinth (0F): lifestock chest in room after key door");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x6F]); // "King Nole's Labyrinth (-1F): lifestock chest in \"small maze\" room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x70]); // "King Nole's Labyrinth (0F): chest in spike balls room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x71]); // "King Nole's Labyrinth (-1F): dark room with three chests");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x72]); // "King Nole's Labyrinth (-1F): dark room with three chests");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x73]); // "King Nole's Labyrinth (-1F): dark room with three chests");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x74]); // "King Nole's Labyrinth (-1F): lifestock chest in \"big maze\" room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x75]); // "King Nole's Labyrinth (-1F): ekeeke chest in lantern room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x76]); // "King Nole's Labyrinth (-1F): lantern chest");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x77]); // "King Nole's Labyrinth (-1F): key chest in ice shortcut room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x78]); // "King Nole's Labyrinth (-2F): ekeeke chest in skeleton priest room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x79]); // "King Nole's Labyrinth (-1F): key chest in \"button and crates\" room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x7A]); // "King Nole's Labyrinth (-3F): ekeeke chest before Firedemon");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x7B]); // "King Nole's Labyrinth (-3F): dahl chest before Firedemon");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x7C]); // "King Nole's Labyrinth (-3F): reward for beating Firedemon");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x7D]); // "King Nole's Labyrinth (-3F): lifestock chest in four buttons room");
	kingNolesLabyrinthEntrance->addItemSource(_chests[0x85]); // "King Nole's Labyrinth (-2F): chest in hands room");
	kingNolesLabyrinthEntrance->addItemSource(_pedestals[ItemPedestalCode::SPIKE_BOOTS]);
	kingNolesLabyrinthEntrance->addItemSource(_pedestals[ItemPedestalCode::IRON_BOOTS]);
	kingNolesLabyrinthEntrance->addItemSource(_pedestals[ItemPedestalCode::GAIA_SWORD]);
	kingNolesLabyrinthEntrance->addItemSource(_pedestals[ItemPedestalCode::VENUS_STONE]);
	_regions.push_back(kingNolesLabyrinthEntrance);

	WorldRegion* kingNolesLabyrinthPostSpikeBoots = new WorldRegion("King Nole's Labyrinth (post spike boots)");
	kingNolesLabyrinthPostSpikeBoots->addItemSource(_chests[0x83]); // "King Nole's Labyrinth (-3F): chest before Miro");
	kingNolesLabyrinthPostSpikeBoots->addItemSource(_chests[0x84]); // "King Nole's Labyrinth (-3F): reward for beating Miro");
	_regions.push_back(kingNolesLabyrinthPostSpikeBoots);

	WorldRegion* kingNolesLabyrinthLogsSector = new WorldRegion("King Nole's Labyrinth (logs sector)");
	kingNolesLabyrinthLogsSector->addItemSource(_pedestals[ItemPedestalCode::LOGS_1]);
	kingNolesLabyrinthLogsSector->addItemSource(_pedestals[ItemPedestalCode::LOGS_2]);
	_regions.push_back(kingNolesLabyrinthLogsSector);

	WorldRegion* kingNolesLabyrinthPostLogs = new WorldRegion("King Nole's Labyrinth (post logs)");
	kingNolesLabyrinthPostLogs->addItemSource(_chests[0x7E]); // "King Nole's Labyrinth (-3F): first ekeeke chest before Spinner");
	kingNolesLabyrinthPostLogs->addItemSource(_chests[0x7F]); // "King Nole's Labyrinth (-3F): second ekeeke chest before Spinner");
//		kingNolesLabyrinthPostLogs->addItemSource(_chests[0x80]); // "King Nole's Labyrinth (-3F): statue of gaia chest before Spinner");
	kingNolesLabyrinthPostLogs->addItemSource(_chests[0x81]); // "King Nole's Labyrinth (-3F): reward for beating Spinner");
	kingNolesLabyrinthPostLogs->addItemSource(_chests[0x82]); // "King Nole's Labyrinth (-3F): key chest in Hyper Breast room");
	kingNolesLabyrinthPostLogs->addItemSource(_pedestals[ItemPedestalCode::HYPER_BREAST]);
	_regions.push_back(kingNolesLabyrinthPostLogs);

	WorldRegion* kingNolesPalace = new WorldRegion("King Nole's Palace");
	kingNolesPalace->addItemSource(_chests[0x13]); // "King Nole's Palace: entrance lifestock chest");
	kingNolesPalace->addItemSource(_chests[0x14]); // "King Nole's Palace: ekeeke chest in topmost pit room");
	kingNolesPalace->addItemSource(_chests[0x15]); // "King Nole's Palace: dahl chest in floating button room");
	_regions.push_back(kingNolesPalace);

	// Create 3 fake regions to require the 3 Gola items
	WorldRegion* endItem1 = new WorldRegion("GO MODE 1/3");
	WorldRegion* endItem2 = new WorldRegion("GO MODE 2/3");
	WorldRegion* endItem3 = new WorldRegion("GO MODE 3/3");
	_regions.push_back(endItem1);
	_regions.push_back(endItem2);
	_regions.push_back(endItem3);

	massan->addPathTo(massanCave, _items[ITEM_AXE_MAGIC]);
	massan->addPathTo(routeMassanToGumi);
	routeMassanToGumi->addPathTo(waterfallShrine);
	routeMassanToGumi->addPathTo(swampShrine, _items[ITEM_IDOL_STONE]);
	routeMassanToGumi->addPathTo(gumi);
	gumi->addPathTo(routeGumiToRyuma);
	routeGumiToRyuma->addPathTo(tibor);
	routeGumiToRyuma->addPathTo(ryuma);
	routeGumiToRyuma->addPathTo(mercator, _items[ITEM_SAFETY_PASS]);
	routeGumiToRyuma->addPathTo(witchHelgaHut, _items[ITEM_EINSTEIN_WHISTLE]);
	ryuma->addPathTo(thievesHideout);
	mercator->addPathTo(mercatorDungeon);
	mercator->addPathTo(crypt);
	mercator->addPathTo(mirTowerSector);
	mercator->addPathTo(mercatorSpecialShop, _items[ITEM_BUYER_CARD]);
	mercator->addPathTo(greenmaze, _items[ITEM_KEY]);
	mercator->addPathTo(verlaShore, _items[ITEM_SUN_STONE]);
	mirTowerSector->addPathTo(mirTowerSectorSacredTrees, _items[ITEM_AXE_MAGIC]);
	mirTowerSector->addPathTo(mirTowerPreGarlic, _items[ITEM_ARMLET]);
	mirTowerPreGarlic->addPathTo(mirTowerPostGarlic, _items[ITEM_GARLIC]);
	verlaShore->addPathTo(verlaMines);
	verlaMines->addPathTo(routeToDestel);
	routeToDestel->addPathTo(destel);
	destel->addPathTo(routeAfterDestel);
	destel->addPathTo(destelWell);
	destelWell->addPathTo(routeToLakeShrine);
	routeToLakeShrine->addPathTo(lakeShrine, _items[ITEM_GAIA_STATUE]);
	//		routeToLakeShrine->addPathTo(lakeShrine, _items[ITEM_GAIA_SWORD]);
	greenmaze->addPathTo(mountainousArea, _items[ITEM_AXE_MAGIC]);
	greenmaze->addPathTo(greenmazeBehindSacredTrees, _items[ITEM_EINSTEIN_WHISTLE]);
	mountainousArea->addPathTo(routeToLakeShrine, _items[ITEM_AXE_MAGIC]);
	mountainousArea->addPathTo(kingNolesCave, _items[ITEM_GOLA_EYE]);
	kingNolesCave->addPathTo(kazalt);
	kazalt->addPathTo(kingNolesLabyrinthEntrance);
	kingNolesLabyrinthEntrance->addPathTo(kingNolesLabyrinthPostSpikeBoots, _items[ITEM_SPIKE_BOOTS]);
	kingNolesLabyrinthPostSpikeBoots->addPathTo(kingNolesLabyrinthLogsSector, _items[ITEM_AXE_MAGIC]);
	kingNolesLabyrinthPostSpikeBoots->addPathTo(kingNolesPalace, _items[ITEM_SPIKE_BOOTS]);
	kingNolesLabyrinthPostSpikeBoots->addPathTo(kingNolesLabyrinthPostLogs, _items[ITEM_LOGS]);
	kingNolesPalace->addPathTo(endItem1, _items[ITEM_GOLA_FANG]);
	endItem1->addPathTo(endItem2, _items[ITEM_GOLA_HORN]);
	endItem2->addPathTo(endItem3, _items[ITEM_GOLA_NAIL]);

	// Set fixed chests
	_chests[0x80]->setItem(_items[ITEM_LOGS]);
}

void World::initFillerItems()
{
	_fillerItems.push_back(_items[ITEM_MAGIC_SWORD]);
	_fillerItems.push_back(_items[ITEM_THUNDER_SWORD]);
	_fillerItems.push_back(_items[ITEM_ICE_SWORD]);
	_fillerItems.push_back(_items[ITEM_GAIA_SWORD]);

	_fillerItems.push_back(_items[ITEM_STEEL_BREAST]);
	_fillerItems.push_back(_items[ITEM_CHROME_BREAST]);
	_fillerItems.push_back(_items[ITEM_SHELL_BREAST]);
	_fillerItems.push_back(_items[ITEM_HYPER_BREAST]);

	_fillerItems.push_back(_items[ITEM_HEALING_BOOTS]);
	_fillerItems.push_back(_items[ITEM_IRON_BOOTS]);
	_fillerItems.push_back(_items[ITEM_FIREPROOF_BOOTS]);

	_fillerItems.push_back(_items[ITEM_MARS_STONE]);
	_fillerItems.push_back(_items[ITEM_MOON_STONE]);
	_fillerItems.push_back(_items[ITEM_SATURN_STONE]);
	_fillerItems.push_back(_items[ITEM_VENUS_STONE]);

	_fillerItems.push_back(_items[ITEM_DEATH_STATUE]);
	_fillerItems.push_back(_items[ITEM_PAWN_TICKET]);
	_fillerItems.push_back(_items[ITEM_BELL]);
	_fillerItems.push_back(_items[ITEM_LANTERN]);
	_fillerItems.push_back(_items[ITEM_BLUE_RIBBON]);
	_fillerItems.push_back(_items[ITEM_GARLIC]);
	_fillerItems.push_back(_items[ITEM_LOGS]);
	_fillerItems.push_back(_items[ITEM_ORACLE_STONE]);

	_fillerItems.push_back(_items[ITEM_SHORT_CAKE]);
	_fillerItems.push_back(_items[ITEM_RED_JEWEL]);
	_fillerItems.push_back(_items[ITEM_PURPLE_JEWEL]);
	_fillerItems.push_back(_items[ITEM_LITHOGRAPH]);
	_fillerItems.push_back(_items[ITEM_SPELL_BOOK]);
	_fillerItems.push_back(_items[ITEM_STATUE_JYPTA]);

	for (uint8_t i = 0; i < 75; ++i)
		_fillerItems.push_back(_items[ITEM_LIFESTOCK]);
	for (uint8_t i = 0; i < 55; ++i)
		_fillerItems.push_back(_items[ITEM_EKEEKE]);
	for (uint8_t i = 0; i < 20; ++i)
		_fillerItems.push_back(_items[ITEM_DAHL]);
	for (uint8_t i = 0; i < 14; ++i)
		_fillerItems.push_back(_items[ITEM_GAIA_STATUE]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_items[ITEM_GOLDEN_STATUE]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_items[ITEM_RESTORATION]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_items[ITEM_DETOX_GRASS]);
	for (uint8_t i = 0; i < 8; ++i)
		_fillerItems.push_back(_items[ITEM_MIND_REPAIR]);
	for (uint8_t i = 0; i < 8; ++i)
		_fillerItems.push_back(_items[ITEM_ANTI_PARALYZE]);

	for (uint8_t i = 0; i < 15; ++i)
		_fillerItems.push_back(_items[ITEM_5_GOLDS]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_items[ITEM_20_GOLDS]);
	for (uint8_t i = 0; i < 8; ++i)
		_fillerItems.push_back(_items[ITEM_50_GOLDS]);
	for (uint8_t i = 0; i < 2; ++i)
		_fillerItems.push_back(_items[ITEM_200_GOLDS]);

	Tools::shuffle(_fillerItems, _rng);
}

void World::randomize()
{
	std::vector<Item*> playerInventory;

	for (int stepCount = 1;  ; ++stepCount)
	{
		// Evaluate accessible regions & stuff
		std::vector<Item*> keyItemsNeededToProgress;
		std::vector<AbstractItemSource*> reachableItemSources;
		std::vector<WorldRegion*> reachableRegions = this->evaluateReachableRegions(playerInventory, keyItemsNeededToProgress, reachableItemSources);
		if (keyItemsNeededToProgress.empty())
			break;

		Tools::shuffle(reachableItemSources, _rng);

		_logFile << "Step #" << stepCount << "\n";

		_logFile << "\t > Accessible regions are: ";
		for (WorldRegion* region : reachableRegions)
			_logFile << region->getName() << ", ";
		_logFile << "\n";

		_logFile << "\t > Key items needed to progress are: ";
		for (Item* item : keyItemsNeededToProgress)
			_logFile << item->getName() << ", ";
		_logFile << "\n";

		// Find a random "key item" to place and a compatible item source where to place it
		Tools::shuffle(keyItemsNeededToProgress, _rng);
		Item* randomKeyItem = keyItemsNeededToProgress[0];

		AbstractItemSource* randomItemSource = nullptr;
		for (uint32_t i = 0; i < reachableItemSources.size() ; ++i)
		{
			if (reachableItemSources[i]->isItemCompatible(randomKeyItem))
			{
				randomItemSource = reachableItemSources[i];
				reachableItemSources.erase(reachableItemSources.begin() + i);
				break;
			}
		}
		if (!randomItemSource)
			throw NoAppropriateItemSourceException();

		_logFile << "\t > Key item is [" << randomKeyItem->getName() << "], putting it in \"" << randomItemSource->getName() << "\"\n";
		randomItemSource->setItem(randomKeyItem);
		playerInventory.push_back(randomKeyItem);

		// Fill additionnal item sources with "filler items"
		int additionnalSourcesToFill = static_cast<int>(reachableItemSources.size() * 0.25);
		_logFile << "\t > Filling " << additionnalSourcesToFill << " additionnal sources with filler items\n";
		this->fillSourcesWithFillerItems(reachableItemSources, additionnalSourcesToFill);
	}

	// If no key items are remaining, this means we are in go-mode and we can fill all remaining item sources
	std::vector<Item*> keyItemsNeededToProgress;
	std::vector<AbstractItemSource*> reachableItemSources;
	std::vector<WorldRegion*> reachableRegions = this->evaluateReachableRegions(playerInventory, keyItemsNeededToProgress, reachableItemSources);

	_logFile << "Key items placement finished, filling the " << reachableItemSources.size() << " remaining sources...\n";
	this->fillSourcesWithFillerItems(reachableItemSources);

	// Write down the complete item list in the log file
	this->writeItemSourcesBreakdownInLog();
}

std::vector<WorldRegion*> World::evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources)
{
	std::vector<WorldRegion*> returnedRegions;
	std::vector<WorldRegion*> regionsToProcess;
	regionsToProcess.push_back(_spawnRegion);

	while (!regionsToProcess.empty())
	{
		WorldRegion* processedRegion = *(regionsToProcess.begin());
		regionsToProcess.erase(regionsToProcess.begin());
		returnedRegions.push_back(processedRegion);

		// Add empty item sources from this region to the reachable item sources
		const std::vector<AbstractItemSource*> itemSourcesInRegion = processedRegion->getItemSources();
		for (AbstractItemSource* source : itemSourcesInRegion)
			if (!source->getItem())
				out_reachableSources.push_back(source);

		// Analyze outgoing paths to check for other regions to explore
		const std::vector<WorldPath*>& outgoingPaths = processedRegion->getOutgoingPaths();
		for (WorldPath* path : outgoingPaths)
		{
			WorldRegion* destination = path->getDestination();

			Item* requiredKeyItem = path->getRequiredItem();
			bool canReachRegion = true;
			if (requiredKeyItem)
			{
				canReachRegion = false;
				for (Item* ownedItem : playerInventory)
				{
					if (ownedItem == requiredKeyItem)
					{
						canReachRegion = true;
						break;
					}
				}
			}

			if (!canReachRegion)
			{
				if( std::find(out_keyItems.begin(), out_keyItems.end(), requiredKeyItem) == out_keyItems.end() )
					out_keyItems.push_back(requiredKeyItem);
			}
			else if ( std::find(returnedRegions.begin(), returnedRegions.end(), destination) == returnedRegions.end() )
				regionsToProcess.push_back(destination);
		}
	}

	return returnedRegions;
}

void World::fillSourcesWithFillerItems(const std::vector<AbstractItemSource*>& itemSources, uint32_t count)
{
	for (uint32_t i = 0; i < count && i < itemSources.size() && !_fillerItems.empty(); ++i)
	{
		AbstractItemSource* randomItemSource = itemSources[i];
		Item* randomFillerItem = nullptr;
		for (uint32_t j = 0; j < _fillerItems.size(); ++j)
		{
			if (randomItemSource->isItemCompatible(_fillerItems[j]))
			{
				randomFillerItem = _fillerItems[j];
				_fillerItems.erase(_fillerItems.begin() + j);
				break;
			}
		}

		if (randomFillerItem)
		{
			randomItemSource->setItem(randomFillerItem);
			_logFile << "\t\t >>> Filling \"" << randomItemSource->getName() << "\" with [" << randomFillerItem->getName() << "]\n";
		}
	}
}


void World::writeToROM(GameROM& rom)
{
	for (auto& [key, item] : _items)
		item->writeToROM(rom);
	for (auto& [key, chest] : _chests)
		chest->writeToROM(rom);
	for (auto& [key, pedestal] : _pedestals)
		pedestal->writeToROM(rom);
	for (auto& [key, reward] : _rewards)
		reward->writeToROM(rom);
}

void World::writeItemSourcesBreakdownInLog()
{
	for (WorldRegion* region : _regions)
	{
		auto sources = region->getItemSources();
		if (sources.empty())
			continue;

		_logFile << "\n-------------------------------\n";
		_logFile << "\t" << region->getName() << "\n\n";

		for (AbstractItemSource* source : sources)
		{
			_logFile << "[" << (source->getItem() ? source->getItem()->getName() : "No item") << "] in \"" << source->getName() << "\"\n";
		}
	}

	_logFile << "\n-------------------------------\n";
	_logFile << "Unplaced items:" << "\n";

	for (Item* item : _fillerItems)
		_logFile << "- [" << item->getName() << "]\n";
}