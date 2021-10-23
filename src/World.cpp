#include "World.hpp"

#include "Enums/ItemCodes.hpp"
#include "Enums/ItemSourceCodes.hpp"
#include "Enums/RegionCodes.hpp"
#include "Item.hpp"
#include "Assets.hpp"
#include "ItemSources.hpp"
#include "GameText.hpp"
#include "Huffman/Symbols.hpp"
#include "Huffman/TextEncoder.hpp"
#include "Exceptions.hpp"
#include "WorldSolver.hpp"

World::World(const RandomizerOptions& options) :
    spawnLocation(options.getSpawnLocation()),
    darkenedRegion(nullptr),
    _options(options)
{
    this->initItems(options);

    this->initChests();
    this->initGroundItems();
    this->initShops();
    this->initNPCRewards();

    this->initRegions();
    this->initRegionPaths(options);

    this->initRegionHints();
    this->initHintSigns(options.fillDungeonSignsWithHints());

    this->initDarkRooms();

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

Item* World::addGoldItem(uint8_t worth)
{
    uint8_t highestGoldItemID = items.rbegin()->first;

    // Try to find an item with the same worth
    for(uint8_t i=ITEM_GOLDS_START ; i<=highestGoldItemID ; ++i)
        if(items[i]->getGoldWorth() == worth)
            return items[i];

    // If we consumed all item IDs, don't add it you fool!
    if(highestGoldItemID == 0xFF)
        return nullptr;

    Item* newItem = new ItemGolds(++highestGoldItemID, worth);
    this->addItem(newItem);
    return newItem;
}

Item* World::getItemByName(const std::string& name) const
{
    if(name.empty())
        return nullptr;

    for (auto& [key, item] : items)
        if(item->getName() == name)
            return item;

    return nullptr;
}

WorldRegion* World::getRegionByName(const std::string& name) const
{
    if(name.empty())
        return nullptr;

    for (auto& [key, region] : regions)
        if(region->getName() == name)
            return region;
    
    return nullptr;
}

WorldRegion* World::getRegionForItem(Item* item)
{
    for (auto& [key, region] : regions)
    {
        std::vector<ItemSource*> sources = region->getItemSources();
        for (ItemSource* source : sources)
        {
            if (source->getItem() == item)
            {
                return region;
            }
        }
    }

    return nullptr;
}

std::vector<ItemSource*> World::getItemSourcesContainingItem(Item* item)
{
	std::vector<ItemSource*> sourcesContainingItem;

	for (auto& [key, region] : regions)
	{
		std::vector<ItemSource*> sources = region->getItemSources();
		for (ItemSource* source : sources)
			if (source->getItem() == item)
				sourcesContainingItem.push_back(source);
    }

    return sourcesContainingItem;
}

void World::writeToROM(md::ROM& rom)
{
    // Write a data block for gold values
    uint8_t highestGoldItemID = items.rbegin()->first;
    uint8_t goldItemsCount = (highestGoldItemID - ITEM_GOLDS_START) + 1;
    uint32_t addr = rom.reserveDataBlock(goldItemsCount, "data_gold_values");
    for(uint8_t itemID = ITEM_GOLDS_START ; itemID <= highestGoldItemID ; ++itemID, ++addr)
        rom.setByte(addr, static_cast<uint8_t>(items[itemID]->getGoldWorth()));

    // Write item info
    for (auto& [key, item] : items)
		item->writeToROM(rom);

    // Write item sources' contents
	for (auto& [key, itemSource] : itemSources)
		itemSource->writeToROM(rom);

    // Alter game text lines
    std::vector<std::string> textLines; 
    loadGameStrings(textLines);

    // Kazalt rejection message
    textLines[0x022] = std::string("Only the bearers of the ") 
        + std::to_string(_options.getJewelCount()) + " jewels\n are worthy of entering\n King Nole's domain...\x1E";

    // Regular sign hints
    for (HintSign* sign : hintSigns)
    {
        std::vector<uint16_t> signTextIDs = sign->getTextIDs();
    	textLines[signTextIDs[0]] = GameText(sign->getText()).getOutput();
        for(auto it=signTextIDs.begin()+1 ; it != signTextIDs.end() ; ++it)
            textLines[*it] = " ";
    }

    //  - Lithograph hint
    std::string fullLithographHint;
    for(const std::string& jewelHint : jewelHints)
    {
        if(!fullLithographHint.empty())
            fullLithographHint += " \x1E";
        fullLithographHint += jewelHint;
    }
    if(fullLithographHint.empty())
        fullLithographHint = "This tablet seems of no use...";
    textLines[0x021] = GameText(fullLithographHint).getOutput();

    //  - Fortune teller hint
    textLines[0x28D] = "\x1CHello dear, let me look at\nwhat your future is made of...\x1E";
    textLines[0x28E] = "\x1CI see... I see...\x1E\n" + GameText(fortuneTellerHint).getOutput();
    //  - "Where is lithograph" hint in King Nole's Cave
    textLines[0x0FD] = GameText(whereIsLithographHint).getOutput();
    //  - Oracle stone hint
    textLines[0x019] = GameText(oracleStoneHint).getOutput();

    // Write all text lines into text banks
    TextEncoder encoder(rom, textLines);

    // Inject dark rooms as a data block
    const std::vector<uint16_t>& darkRooms = darkenedRegion->getDarkRooms();
    uint16_t darkRoomsByteCount = static_cast<uint16_t>(darkRooms.size() + 1) * 0x02;
    uint32_t darkRoomsArrayAddress = rom.reserveDataBlock(darkRoomsByteCount, "data_dark_rooms");
    uint8_t i = 0;
    for (uint16_t roomID : darkRooms)
        rom.setWord(darkRoomsArrayAddress + (i++) * 0x2, roomID);
    rom.setWord(darkRoomsArrayAddress + i * 0x2, 0xFFFF);

    // Write Tibor tree map connections
	for (const TreeMap& treeMap : treeMaps)
		treeMap.writeToROM(rom);
}


void World::initItems(const RandomizerOptions& options)
{
    this->addItem(new Item(ITEM_EKEEKE,            "EkeEke",            9,  20));
    this->addItem(new Item(ITEM_MAGIC_SWORD,       "Magic Sword",       1,  300));
    this->addItem(new Item(ITEM_ICE_SWORD,         "Sword of Ice",      1,  300));
    this->addItem(new Item(ITEM_THUNDER_SWORD,     "Thunder Sword",     1,  500));
    this->addItem(new Item(ITEM_GAIA_SWORD,        "Sword of Gaia",     1,  300));
    this->addItem(new Item(ITEM_FIREPROOF_BOOTS,   "Fireproof",         1,  150));
    this->addItem(new Item(ITEM_IRON_BOOTS,        "Iron Boots",        1,  150));
    this->addItem(new Item(ITEM_HEALING_BOOTS,     "Healing Boots",     1,  300));
    this->addItem(new Item(ITEM_SPIKE_BOOTS,       "Snow Spikes",       1,  400));
    this->addItem(new Item(ITEM_STEEL_BREAST,      "Steel Breast",      1,  300));
    this->addItem(new Item(ITEM_CHROME_BREAST,     "Chrome Breast",     1,  400));
    this->addItem(new Item(ITEM_SHELL_BREAST,      "Shell Breast",      1,  500));
    this->addItem(new Item(ITEM_HYPER_BREAST,      "Hyper Breast",      1,  750));
    this->addItem(new Item(ITEM_MARS_STONE,        "Mars Stone",        1,  150));
    this->addItem(new Item(ITEM_MOON_STONE,        "Moon Stone",        1,  150));
    this->addItem(new Item(ITEM_SATURN_STONE,      "Saturn Stone",      1,  200));
    this->addItem(new Item(ITEM_VENUS_STONE,       "Venus Stone",       1,  300));
    this->addItem(new Item(ITEM_AWAKENING_BOOK,    "Awakening Book",    1,  15));
    this->addItem(new Item(ITEM_DETOX_GRASS,       "Detox Grass",       9,  20));
    this->addItem(new Item(ITEM_GAIA_STATUE,       "Statue of Gaia",    9,  100));
    this->addItem(new Item(ITEM_GOLDEN_STATUE,     "Golden Statue",     9,  200, false));
    this->addItem(new Item(ITEM_MIND_REPAIR,       "Mind Repair",       9,  20));
    this->addItem(new Item(ITEM_CASINO_TICKET,     "Casino Ticket",     1,  50));
    this->addItem(new Item(ITEM_AXE_MAGIC,         "Axe Magic",         1,  400));
    this->addItem(new Item(ITEM_BLUE_RIBBON,       "Blue Ribbon",       1,  50));
    this->addItem(new Item(ITEM_BUYER_CARD,        "Buyer's Card",      1,  150));
    this->addItem(new Item(ITEM_LANTERN,           "Lantern",           1,  100));
    this->addItem(new Item(ITEM_GARLIC,            "Garlic",            1,  200));
    this->addItem(new Item(ITEM_ANTI_PARALYZE,     "Anti Paralyze",     9,  20));
    this->addItem(new Item(ITEM_STATUE_JYPTA,      "Statue of Jypta",   1,  250));
    this->addItem(new Item(ITEM_SUN_STONE,         "Sun Stone",         1,  300));
    this->addItem(new Item(ITEM_ARMLET,            "Armlet",            1,  300));
    this->addItem(new Item(ITEM_EINSTEIN_WHISTLE,  "Einstein Whistle",  1,  200));
    this->addItem(new Item(ITEM_BLUE_JEWEL,        "Blue Jewel",        1,  500));
    this->addItem(new Item(ITEM_YELLOW_JEWEL,      "Yellow Jewel",      1,  500));
    this->addItem(new Item(ITEM_RECORD_BOOK,       "Record Book",       1,  200));
    this->addItem(new Item(ITEM_SPELL_BOOK,        "Spell Book",        1,  200));
    this->addItem(new Item(ITEM_HOTEL_REGISTER,    "Hotel Register",    1,  15));
    this->addItem(new Item(ITEM_ISLAND_MAP,        "Island Map",        0,  0));
    this->addItem(new Item(ITEM_LITHOGRAPH,        "Lithograph",        1,  250));
    this->addItem(new Item(ITEM_RED_JEWEL,         "Red Jewel",         1,  500));
    this->addItem(new Item(ITEM_PAWN_TICKET,       "Pawn Ticket",       9,  100));
    this->addItem(new Item(ITEM_PURPLE_JEWEL,      "Purple Jewel",      1,  500));
    this->addItem(new Item(ITEM_GOLA_EYE,          "Gola's Eye",        1,  400));
    this->addItem(new Item(ITEM_DEATH_STATUE,      "Death Statue",      9,  150));
    this->addItem(new Item(ITEM_DAHL,              "Dahl",              9,  100, false));
    this->addItem(new Item(ITEM_RESTORATION,       "Restoration",       9,  40));
    this->addItem(new Item(ITEM_LOGS,              "Logs",              1,  200));
    this->addItem(new Item(ITEM_ORACLE_STONE,      "Oracle Stone",      1,  250));
    this->addItem(new Item(ITEM_IDOL_STONE,        "Idol Stone",        1,  200));
    this->addItem(new Item(ITEM_KEY,               "Key",               1,  150));
    this->addItem(new Item(ITEM_SAFETY_PASS,       "Safety Pass",       1,  300));
    this->addItem(new Item(ITEM_GREEN_JEWEL,       "Green Jewel",       1,  500));
    this->addItem(new Item(ITEM_BELL,              "Bell",              1,  200));
    this->addItem(new Item(ITEM_SHORT_CAKE,        "Short Cake",        9,  150, false));
    this->addItem(new Item(ITEM_GOLA_NAIL,         "Gola's Nail",       1,  800));
    this->addItem(new Item(ITEM_GOLA_HORN,         "Gola's Horn",       1,  800));
    this->addItem(new Item(ITEM_GOLA_FANG,         "Gola's Fang",       1,  800));
    this->addItem(new Item(ITEM_NO_SWORD,          "Broad Sword",       1,  0));
    this->addItem(new Item(ITEM_NO_ARMOR,          "Leather Breast",    1,  0));
    this->addItem(new Item(ITEM_NO_BOOTS,          "Leather Boots",     1,  0));
    this->addItem(new Item(ITEM_NO_RING,           "No Ring",           1,  0));
    this->addItem(new Item(ITEM_LIFESTOCK,         "Life Stock",        1,  250, false));
    this->addItem(new Item(ITEM_NONE,              "No Item",           0,  0));

    if (options.useArmorUpgrades())
    {
        items[ITEM_STEEL_BREAST]->setGoldWorth(250);
        items[ITEM_CHROME_BREAST]->setGoldWorth(250);
        items[ITEM_SHELL_BREAST]->setGoldWorth(250);
        items[ITEM_HYPER_BREAST]->setGoldWorth(250);
    }

    if (options.consumableRecordBook())
    {
        items[ITEM_RECORD_BOOK]->setMaxQuantity(9);
        uint16_t currentPrice = items[ITEM_RECORD_BOOK]->getGoldWorth();
        items[ITEM_RECORD_BOOK]->setGoldWorth(currentPrice / 5);
    }

    // Process custom starting quantities for items
    const std::map<std::string, uint8_t>& startingItems = options.getStartingItems();
    for(auto& [itemName, quantity] : startingItems)
    {
        Item* item = this->getItemByName(itemName);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set starting quantity of unknown item '" << itemName << "'";
            throw RandomizerException(msg.str());
        }

        item->setStartingQuantity(std::min<uint8_t>(quantity, 9));
    }

    // Process custom item prices
    const std::map<std::string, uint16_t>& itemPrices = options.getItemPrices();
    for(auto& [itemName, price] : itemPrices)
    {
        Item* item = this->getItemByName(itemName);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set starting price of unknown item '" << itemName << "'";
            throw RandomizerException(msg.str());
        }

        item->setGoldWorth(price);
    }

    // Process custom item max quantities
    const std::map<std::string, uint8_t>& itemMaxQuantities = options.getItemMaxQuantities();
    for(auto& [itemName, maxQuantity] : itemMaxQuantities)
    {
        Item* item = this->getItemByName(itemName);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set max quantity of unknown item '" << itemName << "'";
            throw RandomizerException(msg.str());
        }

        item->setMaxQuantity(maxQuantity);
    }

    if(options.getJewelCount() > MAX_INDIVIDUAL_JEWELS)
    {
        items[ITEM_RED_JEWEL]->setName("Kazalt Jewel");
        items[ITEM_RED_JEWEL]->setAllowedOnGround(false);
        items[ITEM_RED_JEWEL]->setMaxQuantity(options.getJewelCount());
    }
}

void World::initChests()
{
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_RIGHT_EKEEKE] =                   new ItemChest(0x00, "Swamp Shrine (0F): ekeeke chest in room to the right, close to door");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_CARPET_KEY] =                     new ItemChest(0x01, "Swamp Shrine (0F): key chest in carpet room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_LIFESTOCK] =                      new ItemChest(0x02, "Swamp Shrine (0F): lifestock chest in room to the left, accessible by falling from upstairs");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FALLING_EKEEKE] =                 new ItemChest(0x03, "Swamp Shrine (0F): ekeeke chest falling from the ceiling when beating the orc");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_0F_FRONT_EKEEKE] =                   new ItemChest(0x04, "Swamp Shrine (0F): ekeeke chest in room connected to second entrance (without idol stone)");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_LOWER_BRIDGES_KEY] =              new ItemChest(0x05, "Swamp Shrine (1F): lower key chest in wooden bridges room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_1F_UPPER_BRIDGES_KEY] =              new ItemChest(0x06, "Swamp Shrine (2F): upper key chest in wooden bridges room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_2F_SPIKE_KEY] =                      new ItemChest(0x07, "Swamp Shrine (2F): key chest in spike room");
    itemSources[ItemSourceCode::CHEST_SWAMP_SHRINE_3F_REWARD] =                         new ItemChest(0x08, "Swamp Shrine (3F): lifestock chest in Fara's room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_LIFESTOCK] =         new ItemChest(0x09, "Mercator Dungeon (-1F): lifestock chest after key door");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_KEY] =               new ItemChest(0x0A, "Mercator Dungeon (-1F): key chest in Moralis's cell");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_LEFT] =  new ItemChest(0x0B, "Mercator Dungeon (-1F): left ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_RIGHT] = new ItemChest(0x0C, "Mercator Dungeon (-1F): right ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASTLE_KITCHEN] =                        new ItemChest(0x0D, "Mercator: castle kitchen chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_SICK_MERCHANT] =                         new ItemChest(0x0E, "Mercator: chest replacing sick merchant in secondary shop backroom");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_LEFT] =        new ItemChest(0x0F, "Mercator Dungeon (1F): left ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_RIGHT] =       new ItemChest(0x10, "Mercator Dungeon (1F): right ekeeke chest in double chest room");
    itemSources[ItemSourceCode::CHEST_MERCATOR_ARTHUR_KEY] =                            new ItemChest(0x11, "Mercator: Arthur key chest in castle tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_LIFESTOCK] =               new ItemChest(0x12, "Mercator Dungeon (4F): chest on top of tower");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_LIFESTOCK] =                            new ItemChest(0x13, "King Nole's Palace: entrance lifestock chest");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_EKEEKE] =                               new ItemChest(0x14, "King Nole's Palace: ekeeke chest in topmost pit room");
    itemSources[ItemSourceCode::CHEST_KN_PALACE_DAHL] =                                 new ItemChest(0x15, "King Nole's Palace: dahl chest in floating button room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_LIFESTOCK] =                        new ItemChest(0x16, "King Nole's Cave: first lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_FARA_REWARD] =                             new ItemChest(0x17, "Massan: chest in elder house after freeing Fara");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_FIRST_GOLD] =                             new ItemChest(0x18, "King Nole's Cave: first gold chest in third room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_GOLD] =                            new ItemChest(0x19, "King Nole's Cave: second gold chest in third room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_LIFESTOCK] =                 new ItemChest(0x1A, "Greenmaze: chest on path to lumberjack");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_LUMBERJACK_WHISTLE] =                   new ItemChest(0x1B, "Greenmaze: chest replacing lumberjack");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_THIRD_GOLD] =                             new ItemChest(0x1C, "King Nole's Cave: gold chest in isolated room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_SECOND_LIFESTOCK] =                       new ItemChest(0x1D, "King Nole's Cave: lifestock chest in crate room");
    itemSources[ItemSourceCode::CHEST_KN_CAVE_BOULDER_CHASE_LIFESTOCK] =                new ItemChest(0x1F, "King Nole's Cave: boulder chase corridor chest");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_ENTRANCE_LIFESTOCK] =            new ItemChest(0x21, "Waterfall Shrine: lifestock chest under entrance (accessible after talking with Prospero)");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_END_LIFESTOCK] =                 new ItemChest(0x22, "Waterfall Shrine: lifestock chest near Prospero");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_GOLDS] =                         new ItemChest(0x23, "Waterfall Shrine: chest in button room");
    itemSources[ItemSourceCode::CHEST_WATERFALL_SHRINE_KEY] =                           new ItemChest(0x24, "Waterfall Shrine: upstairs key chest");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_ENTRANCE_EKEEKE] =                new ItemChest(0x26, "Thieves Hideout: chest in entrance room when water is removed");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_LEFT] =                  new ItemChest(0x29, "Thieves Hideout: right chest in room accessible by falling from miniboss room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MINIBOSS_RIGHT] =                 new ItemChest(0x2A, "Thieves Hideout: left chest in room accessible by falling from miniboss room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_LEFT] =              new ItemChest(0x2B, "Thieves Hideout: left chest in Pockets cell");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_RIGHT] =             new ItemChest(0x2C, "Thieves Hideout: right chest in Pockets cell");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_RIGHT] =              new ItemChest(0x2D, "Thieves Hideout: right chest in room after quick climb trial");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_QUICK_CLIMB_LEFT] =               new ItemChest(0x2E, "Thieves Hideout: left chest in room after quick climb trial");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_FIRST_PLATFORM_ROOM] =            new ItemChest(0x2F, "Thieves Hideout: chest in first platform room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_SECOND_PLATFORM_ROOM] =           new ItemChest(0x30, "Thieves Hideout: chest in second platform room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_MOVING_BALLS_RIDDLE] =            new ItemChest(0x31, "Thieves Hideout: reward chest after moving balls room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_EKEEKE_ROLLING_BOULDER] =         new ItemChest(0x32, "Thieves Hideout: chest near rolling boulder");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_LEFT] =               new ItemChest(0x34, "Thieves Hideout: left chest in double ekeeke chest room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_RIGHT] =              new ItemChest(0x35, "Thieves Hideout: right chest in double ekeeke chest room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_LEFT] =               new ItemChest(0x36, "Thieves Hideout: left chest in room before boss");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_BEFORE_BOSS_RIGHT] =              new ItemChest(0x37, "Thieves Hideout: right chest in room before boss");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LITHOGRAPH] =              new ItemChest(0x38, "Thieves Hideout: lithograph chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_5G] =                      new ItemChest(0x39, "Thieves Hideout: 5 golds chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_50G] =                     new ItemChest(0x3A, "Thieves Hideout: 50 golds chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_LIFESTOCK] =               new ItemChest(0x3B, "Thieves Hideout: lifestock chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_REWARD_EKEEKE] =                  new ItemChest(0x3C, "Thieves Hideout: ekeeke chest in boss reward room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_LEFT] =            new ItemChest(0x42, "Verla Mines: right chest in 'crate on spike' room near entrance");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_CRATE_ON_SPIKEBALL_RIGHT] =           new ItemChest(0x43, "Verla Mines: left chest in 'crate on spike' room near entrance");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_JAR_STAIRCASE_ROOM] =                 new ItemChest(0x44, "Verla Mines: chest on isolated cliff in 'jar staircase' room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_DEX_KEY] =                            new ItemChest(0x45, "Verla Mines: Dex reward chest");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_SLASHER_KEY] =                        new ItemChest(0x46, "Verla Mines: Slasher reward chest");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_LEFT] =                          new ItemChest(0x47, "Verla Mines: left chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_MIDDLE] =                        new ItemChest(0x48, "Verla Mines: middle chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_TRIO_RIGHT] =                         new ItemChest(0x49, "Verla Mines: right chest in 3 chests room");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_RIGHT] =                     new ItemChest(0x4A, "Verla Mines: right chest in button room near elevator shaft leading to Marley");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_ELEVATOR_LEFT] =                      new ItemChest(0x4B, "Verla Mines: left chest in button room near elevator shaft leading to Marley");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_LAVA_WALKING] =                       new ItemChest(0x4C, "Verla Mines: chest in hidden room accessible by lava-walking");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_CRATES_KEY] =                         new ItemChest(0x4D, "Destel Well (0F): 'crates and holes' room key chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_STAIRS_EKEEKE] =                      new ItemChest(0x4E, "Destel Well (1F): ekeeke chest on small stairs");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_NARROW_GROUND_LIFESTOCK] =            new ItemChest(0x4F, "Destel Well (1F): lifestock chest on narrow ground");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_SPIKE_HALLWAY] =                      new ItemChest(0x50, "Destel Well (1F): lifestock chest in spike room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_SIDE_LIFESTOCK] =                  new ItemChest(0x51, "Destel Well (2F): lifestock chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_2F_DAHL] =                            new ItemChest(0x52, "Destel Well (2F): dahl chest");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_RIGHT] =                 new ItemChest(0x53, "Destel Well (2F): right chest in Pockets room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_POCKETS_ROOM_LEFT] =                  new ItemChest(0x54, "Destel Well (2F): left chest in Pockets room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_1] =                        new ItemChest(0x55, "Destel Well (3F): key chest in first trigger room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_2] =                        new ItemChest(0x56, "Destel Well (3F): key chest in giants room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_ARENA_KEY_3] =                        new ItemChest(0x57, "Destel Well (3F): key chest in second trigger room");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_TOP] =                     new ItemChest(0x58, "Destel Well (4F): top chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_LEFT] =                    new ItemChest(0x59, "Destel Well (4F): left chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_DOWN] =                    new ItemChest(0x5A, "Destel Well (4F): down chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_DESTEL_WELL_FINAL_ROOM_RIGHT] =                   new ItemChest(0x5B, "Destel Well (4F): right chest in room before Quake");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_KEY] =               new ItemChest(0x5C, "Lake Shrine (-1F): green golem spinner key chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLDEN_STATUE] =                   new ItemChest(0x5D, "Lake Shrine (-1F): golden statue chest in corridor");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GREEN_SPINNER_LIFESTOCK] =         new ItemChest(0x5E, "Lake Shrine (-1F): green golem spinner lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B1_GOLEM_HOPPING_LIFESTOCK] =         new ItemChest(0x5F, "Lake Shrine (-1F): golem hopping lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_LIFESTOCK_FALLING_FROM_UNICORNS] = new ItemChest(0x60, "Lake Shrine (-2F): middle life stock");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_LIFESTOCK] =           new ItemChest(0x61, "Lake Shrine (-2F): 'throne room' lifestock chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B2_THRONE_ROOM_KEY] =                 new ItemChest(0x62, "Lake Shrine (-2F): 'throne room' key chest");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_WHITE_GOLEMS_CEILING] =            new ItemChest(0x63, "Lake Shrine (-3F): white golems room");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_KEY_NEAR_SWORD] =                  new ItemChest(0x64, "Lake Shrine (-3F): key chest near sword of ice");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_SNAKE_CAGING_RIDDLE] =             new ItemChest(0x65, "Lake Shrine (-3F): chest in snake caging room");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_LIFESTOCK_FALLING_FROM_PLATFORMS]= new ItemChest(0x66, "Lake Shrine (-3F): lifestock chest on central block, obtained by falling from above");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_HALLWAY_TO_DUKE] =                 new ItemChest(0x67, "Lake Shrine (-3F): chest before reaching the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_LEFT] =                     new ItemChest(0x68, "Lake Shrine (-3F): reward chest (left) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_MIDDLE] =                   new ItemChest(0x69, "Lake Shrine (-3F): reward chest (middle) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_RIGHT] =                    new ItemChest(0x6A, "Lake Shrine (-3F): reward chest (right) after beating the duke");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_GOLDEN_SPINNER_KEY] =              new ItemChest(0x6B, "Lake Shrine (-3F): key chest near golden golem spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EXTERIOR_KEY] =                   new ItemChest(0x6C, "King Nole's Labyrinth (0F): key chest in \"outside room\"");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_EKEEKE_AFTER_KEYDOOR] =           new ItemChest(0x6D, "King Nole's Labyrinth (0F): ekeeke chest in room after key door");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_LIFESTOCK_AFTER_KEYDOOR] =        new ItemChest(0x6E, "King Nole's Labyrinth (0F): lifestock chest in room after key door");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SMALL_MAZE_LIFESTOCK] =           new ItemChest(0x6F, "King Nole's Labyrinth (-1F): lifestock chest in \"small maze\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_SPIKE_BALLS_GAIA_STATUE] =        new ItemChest(0x70, "King Nole's Labyrinth (0F): chest in spike balls room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_1] =               new ItemChest(0x71, "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (left side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_2] =               new ItemChest(0x72, "King Nole's Labyrinth (-1F): ekeeke chest in triple chest dark room (right side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_0F_DARK_ROOM_TRIO_3] =               new ItemChest(0x73, "King Nole's Labyrinth (-1F): restoration chest in triple chest dark room (left side)");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_BIG_MAZE_LIFESTOCK] =             new ItemChest(0x74, "King Nole's Labyrinth (-1F): lifestock chest in \"big maze\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN_ROOM_EKEEKE] =            new ItemChest(0x75, "King Nole's Labyrinth (-1F): ekeeke chest in lantern room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_LANTERN] =                        new ItemChest(0x76, "King Nole's Labyrinth (-1F): lantern chest");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B1_ICE_SHORTCUT_KEY] =               new ItemChest(0x77, "King Nole's Labyrinth (-1F): key chest in ice shortcut room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_SAVE_ROOM] =                      new ItemChest(0x78, "King Nole's Labyrinth (-2F): ekeeke chest in skeleton priest room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_BUTTON_CRATES_KEY] =              new ItemChest(0x79, "King Nole's Labyrinth (-1F): key chest in \"button and crates\" room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_EKEEKE] =               new ItemChest(0x7A, "King Nole's Labyrinth (-3F): ekeeke chest before Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_DAHL] =                 new ItemChest(0x7B, "King Nole's Labyrinth (-3F): dahl chest before Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_REWARD] =               new ItemChest(0x7C, "King Nole's Labyrinth (-3F): reward for beating Firedemon");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FOUR_BUTTONS_RIDDLE] =            new ItemChest(0x7D, "King Nole's Labyrinth (-2F): lifestock chest in four buttons room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_1] =               new ItemChest(0x7E, "King Nole's Labyrinth (-3F): first ekeeke chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_EKEEKE_2] =               new ItemChest(0x7F, "King Nole's Labyrinth (-3F): second ekeeke chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_GAIA_STATUE] =            new ItemChest(0x80, "King Nole's Labyrinth (-3F): statue of gaia chest before Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_REWARD] =                 new ItemChest(0x81, "King Nole's Labyrinth (-3F): reward for beating Spinner");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_KEY] =                    new ItemChest(0x82, "King Nole's Labyrinth (-3F): key chest in Hyper Breast room");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_GARLIC] =                    new ItemChest(0x83, "King Nole's Labyrinth (-3F): chest before Miro");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_REWARD] =                    new ItemChest(0x84, "King Nole's Labyrinth (-3F): reward for beating Miro");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B2_DEVIL_HANDS] =                    new ItemChest(0x85, "King Nole's Labyrinth (-3F): chest in hands room");
    itemSources[ItemSourceCode::CHEST_ROUTE_GUMI_RYUMA_LIFESTOCK] =                     new ItemChest(0x86, "Route between Gumi and Ryuma: chest on the way to Swordsman Kado");
    itemSources[ItemSourceCode::CHEST_ROUTE_MASSAN_GUMI_PROMONTORY] =                   new ItemChest(0x87, "Route between Massan and Gumi: chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_GOLDEN_STATUE] =                 new ItemChest(0x88, "Route between Mercator and Verla: golden statue chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_RESTORATION] =                   new ItemChest(0x89, "Route between Mercator and Verla: restoration chest on promontory");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TWINKLE_LIFESTOCK] =             new ItemChest(0x8A, "Route between Mercator and Verla: chest near Friday's village");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_ANGLE_PROMONTORY] =                  new ItemChest(0x8B, "Verla Shore: chest on angle promontory after Verla tunnel");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_CLIFF_CHEST] =                       new ItemChest(0x8C, "Verla Shore: chest on highest promontory after Verla tunnel (accessible through Verla mines)");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_HIDDEN_BUTTON_LIFESTOCK] =       new ItemChest(0x8D, "Route to Mir Tower: chest on promontory accessed by pressing hidden switch");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_DAHL] =                     new ItemChest(0x8E, "Route to Mir Tower: dahl chest behind sacred tree");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_BEHIND_CABIN] =                      new ItemChest(0x8F, "Verla Shore: chest behind cabin");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_BUSHES_DAHL] =                 new ItemChest(0x90, "Route to Destel: chest in map right after Verla mines exit");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_ELEVATOR] =                    new ItemChest(0x91, "Route to Destel: chest in 'elevator' map");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_LIFESTOCK] =                new ItemChest(0x92, "Route to Mir Tower: lifestock chest behind sacred tree");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_HIDDEN_LIFESTOCK] =            new ItemChest(0x93, "Route to Destel: hidden chest in map right before Destel");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_DAHL_NEAR_TREE] =                new ItemChest(0x94, "Mountainous Area: chest near teleport tree");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LIFESTOCK_BEFORE_BRIDGE] =       new ItemChest(0x95, "Mountainous Area: chest on right side of the map right before the bridge");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GOLDEN_STATUE] =          new ItemChest(0x96, "Mountainous Area: hidden chest in narrow path");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GAIA_STATUE] =            new ItemChest(0x97, "Mountainous Area: hidden Statue of Gaia chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_CLIFF_LIFESTOCK] =        new ItemChest(0x98, "Mountainous Area: isolated life stock in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_LEFT] =              new ItemChest(0x99, "Mountainous Area: left chest on wall in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BRIDGE_WALL_RIGHT] =             new ItemChest(0x9A, "Mountainous Area: right chest on wall in bridge map");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_RESTORATION_NEAR_ZAK] =          new ItemChest(0x9B, "Mountainous Area: restoration chest in map before Zak arena");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_ZAK] =               new ItemChest(0x9C, "Mountainous Area: ekeeke chest in map before Zak arena");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_CORNER_LIFESTOCK] =            new ItemChest(0x9D, "Route after Destel: chest on cliff angle");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_HIDDEN_LIFESTOCK] =            new ItemChest(0x9E, "Route after Destel: lifestock chest in map after seeing Duke raft");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_DAHL] =                        new ItemChest(0x9F, "Route after Destel: dahl chest in map after seeing Duke raft");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BELOW_ROCKY_ARCH] =              new ItemChest(0xA0, "Mountainous Area: chest hidden under rocky arch");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_EASY_LIFESTOCK] =               new ItemChest(0xA1, "Route to Lake Shrine: 'easy' chest on crossroads with mountainous area");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_LAKE_SHRINE_SHORTCUT] =          new ItemChest(0xA2, "Route to Lake Shrine: 'hard' chest on crossroads with mountainous area");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_EKEEKE_NEAR_BRIDGE] =            new ItemChest(0xA3, "Mountainous Area: chest in map in front of the statue under the bridge");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_RIGHT] =                new ItemChest(0xA4, "Route to Lake Shrine: right chest in volcano");
    itemSources[ItemSourceCode::CHEST_ROUTE_LAKE_SHRINE_VOLCANO_LEFT] =                 new ItemChest(0xA5, "Route to Lake Shrine: left chest in volcano");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN] =                   new ItemChest(0xA6, "Mountainous Area Cave: chest in small hidden room");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_VISIBLE] =                  new ItemChest(0xA7, "Mountainous Area Cave: chest in small visible room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_PROMONTORY_GOLDS] =                     new ItemChest(0xA9, "Greenmaze: chest on promontory appearing after pressing a button in other section");
    itemSources[ItemSourceCode::CHEST_MASSAN_SHORTCUT_DAHL] =                           new ItemChest(0xAA, "Greenmaze: chest between Sunstone and Massan shortcut");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_MAGES_LIFESTOCK] =                      new ItemChest(0xAB, "Greenmaze: chest in mages room");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_LEFT] =                      new ItemChest(0xAC, "Greenmaze: left chest in elbow cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_ELBOW_CAVE_RIGHT] =                     new ItemChest(0xAD, "Greenmaze: right chest in elbow cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_DAHL] =                  new ItemChest(0xAE, "Greenmaze: chest in waterfall cave");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_LIFESTOCK] =             new ItemChest(0xAF, "Greenmaze: left chest in hidden room behind waterfall");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_GOLDS] =                 new ItemChest(0xB0, "Greenmaze: right chest in hidden room behind waterfall");
    itemSources[ItemSourceCode::CHEST_MASSAN_DOG_STATUE] =                              new ItemChest(0xB1, "Massan: chest triggered by dog statue");
    itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_1] =                                new ItemChest(0xB2, "Massan: chest in house nearest to elder house");
    itemSources[ItemSourceCode::CHEST_MASSAN_HOUSE_LIFESTOCK] =                         new ItemChest(0xB3, "Massan: lifestock chest in house");
    itemSources[ItemSourceCode::CHEST_MASSAN_EKEEKE_2] =                                new ItemChest(0xB4, "Massan: chest in house farthest from elder house");
    itemSources[ItemSourceCode::CHEST_GUMI_LIFESTOCK] =                                 new ItemChest(0xB5, "Gumi: chest on top of bed in house");
    itemSources[ItemSourceCode::CHEST_GUMI_FARA_REWARD] =                               new ItemChest(0xB6, "Gumi: chest in elder house after saving Fara");
    itemSources[ItemSourceCode::CHEST_RYUMA_MAYOR_LIFESTOCK] =                          new ItemChest(0xB7, "Ryuma: chest in mayor's house");
    itemSources[ItemSourceCode::CHEST_RYUMA_LIGHTHOUSE_LIFESTOCK] =                     new ItemChest(0xB8, "Ryuma: chest in repaired lighthouse");
    itemSources[ItemSourceCode::CHEST_CRYPT_MAIN_LOBBY] =                               new ItemChest(0xB9, "Crypt: chest in main room");
    itemSources[ItemSourceCode::CHEST_CRYPT_ARMLET] =                                   new ItemChest(0xBA, "Crypt: reward chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO] =                                new ItemChest(0xBF, "Mercator: hidden casino chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_GREENPEA] =                              new ItemChest(0xC0, "Mercator: chest in Greenpea's house");
    itemSources[ItemSourceCode::CHEST_MERCATOR_GRANDMA_POT_SHELVING] =                  new ItemChest(0xC1, "Mercator: chest in grandma's house (pot shelving trial)");
    itemSources[ItemSourceCode::CHEST_VERLA_WELL] =                                     new ItemChest(0xC2, "Verla: chest in well after beating Marley");
    itemSources[ItemSourceCode::CHEST_DESTEL_INN_COUNTER] =                             new ItemChest(0xC4, "Destel: chest in shop requiring to wait for the shopkeeper to move");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_GARLIC] =                               new ItemChest(0xC5, "Mir Tower: garlic chest");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_AFTER_MIMICS] =                  new ItemChest(0xC6, "Mir Tower: chest after mimic room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_1] =                              new ItemChest(0xC7, "Mir Tower: mimic room empty chest 1");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_2] =                              new ItemChest(0xC8, "Mir Tower: mimic room empty chest 2");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_3] =                              new ItemChest(0xC9, "Mir Tower: mimic room empty chest 3");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EMPTY_4] =                              new ItemChest(0xCA, "Mir Tower: mimic room empty chest 4");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_MUSHROOM_PIT_ROOM] =                    new ItemChest(0xCB, "Mir Tower: chest in mushroom pit room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_GAIA_STATUE] =                          new ItemChest(0xCC, "Mir Tower: chest in room next to mummy switch room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_LIBRARY_LIFESTOCK] =                    new ItemChest(0xCD, "Mir Tower: chest in library accessible from teleporter maze");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_HIDDEN_EKEEKE] =                        new ItemChest(0xCE, "Mir Tower: hidden chest in room before library");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_FALLING_SPIKEBALLS] =                   new ItemChest(0xCF, "Mir Tower: chest in falling spikeballs room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_TIMED_KEY] =                            new ItemChest(0xD0, "Mir Tower: chest in timed challenge room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_1] =                  new ItemChest(0xD1, "Mir Tower: chest in room where Miro closes the door");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_EKEEKE_MIRO_CHASE_2] =                  new ItemChest(0xD2, "Mir Tower: chest after room where Miro closes the door");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_PURPLE_JEWEL] =                  new ItemChest(0xD3, "Mir Tower: reward chest");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_RIGHT_EKEEKE] =                  new ItemChest(0xD4, "Mir Tower: right chest in reward room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LEFT_EKEEKE] =                   new ItemChest(0xD5, "Mir Tower: left chest in reward room");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LIFESTOCK] =                     new ItemChest(0xD6, "Mir Tower: chest behind wall accessible after beating Mir");
    itemSources[ItemSourceCode::CHEST_HELGA] =                                          new ItemChest(0xD7, "Witch Helga's Hut: lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_LIFESTOCK] =                          new ItemChest(0xD8, "Massan Cave: lifestock chest");
    itemSources[ItemSourceCode::CHEST_MASSAN_CAVE_DAHL] =                               new ItemChest(0xD9, "Massan Cave: dahl chest");
    itemSources[ItemSourceCode::CHEST_TIBOR_LIFESTOCK] =                                new ItemChest(0xDA, "Tibor: reward chest after boss");
    itemSources[ItemSourceCode::CHEST_TIBOR_SPIKEBALLS_ROOM] =                          new ItemChest(0xDB, "Tibor: chest in spike balls room");
    itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_LEFT] =                                new ItemChest(0xDC, "Tibor: left chest on 2 chest group");
    itemSources[ItemSourceCode::CHEST_TIBOR_DUAL_RIGHT] =                               new ItemChest(0xDD, "Tibor: right chest on 2 chest group");

    // The following chests are absent from the game on release or modded out of the game for the rando, and their IDs are therefore free:
    // 0x0E, 0x1E, 0x20, 0x25, 0x27, 0x28, 0x33, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0xA8, 0xBB, 0xBC, 0xBD, 0xBE, 0xC3
}

void World::initGroundItems()
{
    itemSources[ItemSourceCode::GROUND_IDOL_STONE] =                new ItemOnGround(0x021167, "Gumi: Idol Stone on ground");
    itemSources[ItemSourceCode::GROUND_SUN_STONE] =                 new ItemOnGround(0x020C21, "Greenmaze: Sun Stone on ground");
    itemSources[ItemSourceCode::GROUND_CHROME_BREAST] =             new ItemOnGround(0x01DDB7, "Verla Mines: Chrome Breast on ground");
    itemSources[ItemSourceCode::GROUND_SHELL_BREAST] =              new ItemOnGround(0x01EC99, "Lake Shrine (-3F): Shell Breast on ground");
    itemSources[ItemSourceCode::GROUND_HYPER_BREAST] =              new ItemOnGround(0x01F9BD, "King Nole's Labyrinth (-3F): Hyper Breast on ground");
    itemSources[ItemSourceCode::GROUND_HEALING_BOOTS] =             new ItemOnGround(0x01E247, "Destel Well: Healing Boots on ground");
    itemSources[ItemSourceCode::GROUND_IRON_BOOTS] =                new ItemOnGround(0x01F36F, "King Nole's Labyrinth (-1F): Iron Boots on ground");
    itemSources[ItemSourceCode::GROUND_FIREPROOF] =                 new ItemOnGround(0x022C23, "Massan Cave: Fireproof Boots on ground");
    itemSources[ItemSourceCode::GROUND_SPIKE_BOOTS] =               new ItemOnGround(0x01FAC1, "King Nole's Labyrinth (-3F): Snow Spikes on ground");
    itemSources[ItemSourceCode::GROUND_GAIA_SWORD] =                new ItemOnGround(0x01F183, "King Nole's Labyrinth (-2F): Sword of Gaia on ground");
    itemSources[ItemSourceCode::GROUND_MARS_STONE] =                new ItemOnGround(0x020419, "Route after Destel: Mars Stone on ground");
    itemSources[ItemSourceCode::GROUND_MOON_STONE] =                new ItemOnGround(0x020AED, "Mountainous Area cave: Moon Stone on ground");
    itemSources[ItemSourceCode::GROUND_SATURN_STONE] =              new ItemOnGround(0x0203AB, "Witch Helga's Hut: Saturn Stone on ground");
    itemSources[ItemSourceCode::GROUND_VENUS_STONE] =               new ItemOnGround(0x01F8D7, "King Nole's Labyrinth (-3F): Venus Stone on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_1] =      new ItemOnGround(0x01E873, "Lake Shrine (-2F): north EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_2] =      new ItemOnGround(0x01E87B, "Lake Shrine (-2F): south EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_3] =      new ItemOnGround(0x01E883, "Lake Shrine (-2F): west EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_LAKE_SHRINE_EKEEKE_4] =      new ItemOnGround(0x01E88B, "Lake Shrine (-2F): east EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_ICE_SWORD] =                 new ItemOnGround({ 0x01EE37, 0x01EE41 }, "Lake Shrine (-3F): Sword of Ice on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_1] =  new ItemOnGround(0x02011D, "Twinkle Village: first EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_2] =  new ItemOnGround(0x020115, "Twinkle Village: second EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_3] =  new ItemOnGround(0x02010D, "Twinkle Village: third EkeEke on ground");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_EKEEKE] =          new ItemOnGround(0x0226A3, "Mir Tower: EkeEke on ground in priest room");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_DETOX] =           new ItemOnGround(0x02269B, "Mir Tower: Detox Grass on ground in priest room");
    itemSources[ItemSourceCode::GROUND_MIR_TOWER_RECORD_BOOK] =     new ItemOnGround(0x022673, "Mir Tower: Record Book on ground in priest room");
    itemSources[ItemSourceCode::GROUND_LOGS_1] =                    new ItemOnGround(0x01FA43, "King Nole's Labyrinth (-2F): left Logs on ground");
    itemSources[ItemSourceCode::GROUND_LOGS_2] =                    new ItemOnGround(0x01FA3B, "King Nole's Labyrinth (-2F): right Logs on ground");
    itemSources[ItemSourceCode::GROUND_FIREDEMON_EKEEKE] =          new ItemOnGround(0x01F8E1, "King Nole's Labyrinth (-3F): EkeEke on ground before Firedemon");

    // This ground item is special in the sense that it can only be taken once, so we take advantage of this to consider it as a shop.
    // It will allow putting special items usually not allowed on ground (e.g. Lifestock) inside.
    itemSources[ItemSourceCode::GROUND_FALLING_RIBBON] =            new ItemInShop(0x01BFDF, "Mercator: falling ribbon in castle court", nullptr);
}

void World::initShops()
{
    ItemShop* massanShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MASSAN_LIFESTOCK] =        new ItemInShop(0x02101D, "Massan shop: Life Stock slot", massanShop);
    itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_1] =         new ItemInShop(0x021015, "Massan shop: first EkeEke slot", massanShop);
    itemSources[ItemSourceCode::SHOP_MASSAN_EKEEKE_2] =         new ItemInShop(0x02100D, "Massan shop: second EkeEke slot", massanShop);
    shops.push_back(massanShop);

    ItemShop* gumiInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_GUMI_LIFESTOCK] =          new ItemInShop(0x0211E5, "Gumi shop: Life Stock slot", gumiInn);
    itemSources[ItemSourceCode::SHOP_GUMI_EKEEKE] =             new ItemInShop(0x0211D5, "Gumi shop: EkeEke slot", gumiInn);
    shops.push_back(gumiInn);

    ItemShop* ryumaShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_RYUMA_LIFESTOCK] =         new ItemInShop(0x0212D9, "Ryuma shop: Life Stock slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_GAIA_STATUE] =       new ItemInShop(0x0212C9, "Ryuma shop: Statue of Gaia slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_GOLDEN_STATUE] =     new ItemInShop(0x0212B9, "Ryuma shop: Golden Statue slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_EKEEKE] =            new ItemInShop(0x0212D1, "Ryuma shop: EkeEke slot", ryumaShop);
    itemSources[ItemSourceCode::SHOP_RYUMA_DETOX_GRASS] =       new ItemInShop(0x0212C1, "Ryuma shop: Detox Grass slot", ryumaShop);
    shops.push_back(ryumaShop);

    ItemShop* ryumaInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_RYUMA_INN_EKEEKE] =        new ItemInShop(0x02139F, "Ryuma inn: EkeEke slot", ryumaInn);
    shops.push_back(ryumaInn);

    ItemShop* mercatorShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_ARMOR] =          new ItemInShop(0x021B7B, "Mercator shop: Steel Breast slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_BELL] =           new ItemInShop(0x021B83, "Mercator shop: Bell slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_EKEEKE] =         new ItemInShop(0x021B73, "Mercator shop: EkeEke slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DETOX_GRASS] =    new ItemInShop(0x021B6B, "Mercator shop: Detox Grass slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_GAIA_STATUE] =    new ItemInShop(0x021B63, "Mercator shop: Statue of Gaia slot", mercatorShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_GOLDEN_STATUE] =  new ItemInShop(0x021B5B, "Mercator shop: Golden Statue slot", mercatorShop);
    shops.push_back(mercatorShop);

    ItemShop* mercatorDocksShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_1] = new ItemInShop({ 0x0216BD, 0x02168B }, "Mercator docks shop: left EkeEke slot", mercatorDocksShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_2] = new ItemInShop({ 0x0216C5, 0x021693 }, "Mercator docks shop: middle EkeEke slot", mercatorDocksShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_3] = new ItemInShop({ 0x0216CD, 0x02169B }, "Mercator docks shop: right EkeEke slot", mercatorDocksShop);
    shops.push_back(mercatorDocksShop);

    ItemShop* mercatorSpecialShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_MIND_REPAIR] =    new ItemInShop(0x021CDF, "Mercator special shop: Mind Repair slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_ANTIPARALYZE] =   new ItemInShop(0x021CD7, "Mercator special shop: Anti Paralyze slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_DAHL] =           new ItemInShop(0x021CCF, "Mercator special shop: Dahl slot", mercatorSpecialShop);
    itemSources[ItemSourceCode::SHOP_MERCATOR_SPECIAL_RESTORATION] =    new ItemInShop(0x021CC7, "Mercator special shop: Restoration slot", mercatorSpecialShop);
    shops.push_back(mercatorSpecialShop);

    ItemShop* verlaShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_VERLA_LIFESTOCK] =         new ItemInShop(0x021F57, "Verla shop: Life Stock slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_EKEEKE] =            new ItemInShop({ 0x021F05, 0x021F37 }, "Verla shop: EkeEke slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_DETOX_GRASS] =       new ItemInShop({ 0x021F0D, 0x021F3F }, "Verla shop: Detox Grass slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_DAHL] =              new ItemInShop({ 0x021F15, 0x021F47 }, "Verla shop: Dahl slot", verlaShop);
    itemSources[ItemSourceCode::SHOP_VERLA_MAP] =               new ItemInShop({ 0x021F25, 0x021F4F }, "Verla shop: Map slot", verlaShop);
    shops.push_back(verlaShop);

    ItemShop* kelketoShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_KELKETO_LIFESTOCK] =       new ItemInShop(0x020861, "Kelketo Waterfalls shop: Life Stock slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_EKEEKE] =          new ItemInShop(0x020869, "Kelketo Waterfalls shop: EkeEke slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_DETOX_GRASS] =     new ItemInShop(0x020871, "Kelketo Waterfalls shop: Detox Grass slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_DAHL] =            new ItemInShop(0x020879, "Kelketo Waterfalls shop: Dahl slot", kelketoShop);
    itemSources[ItemSourceCode::SHOP_KELKETO_RESTORATION] =     new ItemInShop(0x020881, "Kelketo Waterfalls shop: Restoration slot", kelketoShop);
    shops.push_back(kelketoShop);

    ItemShop* destelInn = new ItemShop();
    itemSources[ItemSourceCode::SHOP_DESTEL_INN_EKEEKE] =       new ItemInShop(0x022017, "Destel inn: EkeEke slot", destelInn);
    shops.push_back(destelInn);

    ItemShop* destelShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_DESTEL_EKEEKE] =           new ItemInShop(0x022055, "Destel shop: EkeEke slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_DETOX_GRASS] =      new ItemInShop(0x02206D, "Destel shop: Detox Grass slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_RESTORATION] =      new ItemInShop(0x022065, "Destel shop: Restoration slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_DAHL] =             new ItemInShop(0x02205D, "Destel shop: Dahl slot", destelShop);
    itemSources[ItemSourceCode::SHOP_DESTEL_LIFE_STOCK] =       new ItemInShop(0x022075, "Destel shop: Life Stock slot", destelShop);
    shops.push_back(destelShop);

    ItemShop* greedlyShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_GREEDLY_GAIA_STATUE] =     new ItemInShop(0x0209C7, "Greedly's shop: Statue of Gaia slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_GOLDEN_STATUE] =   new ItemInShop(0x0209BF, "Greedly's shop: Golden Statue slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_DAHL] =            new ItemInShop(0x0209CF, "Greedly's shop: Dahl slot", greedlyShop);
    itemSources[ItemSourceCode::SHOP_GREEDLY_LIFE_STOCK] =      new ItemInShop(0x0209AF, "Greedly's shop: Life Stock slot", greedlyShop);
    shops.push_back(greedlyShop);

    ItemShop* kazaltShop = new ItemShop();
    itemSources[ItemSourceCode::SHOP_KAZALT_EKEEKE] =           new ItemInShop(0x022115, "Kazalt shop: EkeEke slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_DAHL] =             new ItemInShop(0x022105, "Kazalt shop: Dahl slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_GOLDEN_STATUE] =    new ItemInShop(0x02211D, "Kazalt shop: Golden Statue slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_RESTORATION] =      new ItemInShop(0x02210D, "Kazalt shop: Restoration slot", kazaltShop);
    itemSources[ItemSourceCode::SHOP_KAZALT_LIFESTOCK] =        new ItemInShop(0x0220F5, "Kazalt shop: Life Stock slot", kazaltShop);
    shops.push_back(kazaltShop);
}

void World::initNPCRewards()
{
    itemSources[ItemSourceCode::NPC_MIR_AXE_MAGIC] =            new ItemReward(0x028A3F, "Mir reward after Lake Shrine (Axe Magic in OG)");
    itemSources[ItemSourceCode::NPC_ZAK_GOLA_EYE] =             new ItemReward(0x028A73, "Zak reward after fighting (Gola's Eye in OG)");
    itemSources[ItemSourceCode::NPC_KADO_MAGIC_SWORD] =         new ItemReward(0x02894B, "Swordman Kado reward (Magic Sword in OG)");
    itemSources[ItemSourceCode::NPC_HIDDEN_DWARF_RESTORATION] = new ItemReward(0x0288DF, "Greenmaze hidden dwarf (Restoration in OG)");
    itemSources[ItemSourceCode::NPC_ARTHUR_CASINO_TICKET] =     new ItemReward(0x02815F, "Arthur reward (Casino Ticket in OG)");
    itemSources[ItemSourceCode::NPC_FAHL_PAWN_TICKET] =         new ItemReward(0x0284A5, "Fahl's dojo challenge reward (Pawn Ticket in OG)");
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

    regions[RegionCode::MASSAN_AFTER_SWAMP_SHRINE] = new WorldRegion("Massan (after Swamp Shrine)", { 
        itemSources[ItemSourceCode::CHEST_MASSAN_FARA_REWARD]
    });

    regions[RegionCode::GUMI_AFTER_SWAMP_SHRINE] = new WorldRegion("Gumi (after Swamp Shrine)", { 
        itemSources[ItemSourceCode::CHEST_GUMI_FARA_REWARD]
    });

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

    regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE] = new WorldRegion("Ryuma (repaired lighthouse)", { 
        itemSources[ItemSourceCode::CHEST_RYUMA_LIGHTHOUSE_LIFESTOCK]
    });

    regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY] = new WorldRegion("Thieves Hideout (before keydoor)", { 
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_EKEEKE_ROLLING_BOULDER],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_DUAL_EKEEKE_RIGHT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_ENTRANCE_EKEEKE],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_LEFT],
        itemSources[ItemSourceCode::CHEST_THIEVES_HIDEOUT_POCKETS_CELL_RIGHT],
    });
    
    regions[RegionCode::THIEVES_HIDEOUT_POST_KEY] = new WorldRegion("Thieves Hideout (after keydoor)", { 
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
        itemSources[ItemSourceCode::GROUND_FALLING_RIBBON],
        itemSources[ItemSourceCode::SHOP_MERCATOR_ARMOR],
        itemSources[ItemSourceCode::SHOP_MERCATOR_BELL],
        itemSources[ItemSourceCode::SHOP_MERCATOR_EKEEKE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DETOX_GRASS],
        itemSources[ItemSourceCode::SHOP_MERCATOR_GAIA_STATUE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_GOLDEN_STATUE],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_1],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_2],
        itemSources[ItemSourceCode::SHOP_MERCATOR_DOCKS_EKEEKE_3],
        itemSources[ItemSourceCode::NPC_ARTHUR_CASINO_TICKET],
        itemSources[ItemSourceCode::NPC_FAHL_PAWN_TICKET]
    });

    regions[RegionCode::MERCATOR_CASINO] = new WorldRegion("Mercator casino", {
        itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO]
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
        itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_KEY],
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
    });

    regions[RegionCode::MIR_TOWER_SECTOR_BEHIND_TREES] = new WorldRegion("Mir Tower sector (behind trees)", {
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_DAHL],
        itemSources[ItemSourceCode::CHEST_MIR_TOWER_SECTOR_TREE_LIFESTOCK]
    });

    regions[RegionCode::TWINKLE_VILLAGE] = new WorldRegion("Twinkle village", {
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_1],
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_2],
        itemSources[ItemSourceCode::GROUND_TWINKLE_VILLAGE_EKEEKE_3]
    });

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

    regions[RegionCode::GREENMAZE_PRE_WHISTLE] = new WorldRegion("Greenmaze (pre-whistle)", {
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

    regions[RegionCode::GREENMAZE_POST_WHISTLE] = new WorldRegion("Greenmaze (post-whistle)", {
        itemSources[ItemSourceCode::GROUND_SUN_STONE],
        itemSources[ItemSourceCode::CHEST_MASSAN_SHORTCUT_DAHL]
        });

    regions[RegionCode::VERLA_SECTOR] = new WorldRegion("Verla sector", {
        itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_BEHIND_CABIN],
        itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_ANGLE_PROMONTORY],
        itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_CLIFF_CHEST],
    });

    regions[RegionCode::VERLA] = new WorldRegion("Verla", {
        itemSources[ItemSourceCode::CHEST_VERLA_WELL],
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
	    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_REWARD],
        itemSources[ItemSourceCode::GROUND_LOGS_1],
        itemSources[ItemSourceCode::GROUND_LOGS_2]
    });

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

    macroRegions = {
        new WorldMacroRegion("the village of Massan",	{ regions[RegionCode::MASSAN], regions[RegionCode::MASSAN_AFTER_SWAMP_SHRINE] }),
        new WorldMacroRegion("the cave near Massan",	{ regions[RegionCode::MASSAN_CAVE] }),
        new WorldMacroRegion("the waterfall shrine",	{ regions[RegionCode::WATERFALL_SHRINE] }),
        new WorldMacroRegion("the village of Gumi",		{ regions[RegionCode::GUMI], regions[RegionCode::GUMI_AFTER_SWAMP_SHRINE] }),
        new WorldMacroRegion("the swamp shrine",		{ regions[RegionCode::SWAMP_SHRINE] }),
        new WorldMacroRegion("Tibor",					{ regions[RegionCode::TIBOR] }),
        new WorldMacroRegion("the town of Ryuma",		{ regions[RegionCode::RYUMA], regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE] }),
        new WorldMacroRegion("the thieves' hideout",	{ regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY], regions[RegionCode::THIEVES_HIDEOUT_POST_KEY] }),
        new WorldMacroRegion("witch Helga's hut",		{ regions[RegionCode::WITCH_HELGA_HUT] }),
        new WorldMacroRegion("the town of Mercator",	{ regions[RegionCode::MERCATOR], regions[RegionCode::MERCATOR_CASINO], regions[RegionCode::MERCATOR_SPECIAL_SHOP] }),
        new WorldMacroRegion("the crypt of Mercator",	{ regions[RegionCode::CRYPT] }),
        new WorldMacroRegion("the dungeon of Mercator",	{ regions[RegionCode::MERCATOR_DUNGEON] }),
        new WorldMacroRegion("Mir Tower",				{ regions[RegionCode::MIR_TOWER_PRE_GARLIC], regions[RegionCode::MIR_TOWER_POST_GARLIC] }),
        new WorldMacroRegion("Greenmaze",				{ regions[RegionCode::GREENMAZE_PRE_WHISTLE], regions[RegionCode::GREENMAZE_POST_WHISTLE] }),
        new WorldMacroRegion("the town of Verla",		{ regions[RegionCode::VERLA] }),
        new WorldMacroRegion("Verla mine",				{ regions[RegionCode::VERLA_MINES] }),
        new WorldMacroRegion("the village of Destel",	{ regions[RegionCode::DESTEL] }),
        new WorldMacroRegion("Destel well",				{ regions[RegionCode::DESTEL_WELL] }),
        new WorldMacroRegion("the lake shrine",			{ regions[RegionCode::LAKE_SHRINE] }),
        new WorldMacroRegion("the mountainous area",	{ regions[RegionCode::MOUNTAINOUS_AREA] }),
        new WorldMacroRegion("King Nole's cave",		{ regions[RegionCode::KN_CAVE] }),
        new WorldMacroRegion("the town of Kazalt",		{ regions[RegionCode::KAZALT] }),
        new WorldMacroRegion("King Nole's labyrinth",	{ regions[RegionCode::KN_LABYRINTH_PRE_SPIKES], regions[RegionCode::KN_LABYRINTH_POST_SPIKES], regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR] }),
        new WorldMacroRegion("King Nole's palace",		{ regions[RegionCode::KN_PALACE] })
    };
}

void World::initRegionPaths(const RandomizerOptions& options)
{
    // Determine the list of required jewels to go from King Nole's Cave to Kazalt depending on settings
    std::vector<Item*> requiredJewels;
    if(options.getJewelCount() > MAX_INDIVIDUAL_JEWELS)
    {
        for(int i=0; i<options.getJewelCount() ; ++i)
            requiredJewels.push_back(items[ITEM_RED_JEWEL]);
    }
    else if(options.getJewelCount() >= 1)
    {
        requiredJewels.push_back(items[ITEM_RED_JEWEL]);
        if(options.getJewelCount() >= 2)
            requiredJewels.push_back(items[ITEM_PURPLE_JEWEL]);
        if(options.getJewelCount() >= 3)
            requiredJewels.push_back(items[ITEM_GREEN_JEWEL]);
        if(options.getJewelCount() >= 4)
            requiredJewels.push_back(items[ITEM_BLUE_JEWEL]);
        if(options.getJewelCount() >= 5)
            requiredJewels.push_back(items[ITEM_YELLOW_JEWEL]);
    }

    // Create region paths with item conditions
	regions[RegionCode::MASSAN]->addPathsBetween(regions[RegionCode::MASSAN_CAVE], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::MASSAN]->addPathsBetween(regions[RegionCode::ROUTE_MASSAN_GUMI]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathsBetween(regions[RegionCode::WATERFALL_SHRINE]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathsBetween(regions[RegionCode::SWAMP_SHRINE], items[ITEM_IDOL_STONE], 2);
    regions[RegionCode::SWAMP_SHRINE]->addPathsBetween(regions[RegionCode::MASSAN_AFTER_SWAMP_SHRINE]);
    regions[RegionCode::SWAMP_SHRINE]->addPathsBetween(regions[RegionCode::GUMI_AFTER_SWAMP_SHRINE]);
	regions[RegionCode::ROUTE_MASSAN_GUMI]->addPathsBetween(regions[RegionCode::GUMI]);
	regions[RegionCode::GUMI]->addPathsBetween(regions[RegionCode::ROUTE_GUMI_RYUMA]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathsBetween(regions[RegionCode::TIBOR]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathsBetween(regions[RegionCode::RYUMA]);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathsBetween(regions[RegionCode::MERCATOR], items[ITEM_SAFETY_PASS], 2);
	regions[RegionCode::ROUTE_GUMI_RYUMA]->addPathsBetween(regions[RegionCode::WITCH_HELGA_HUT], items[ITEM_EINSTEIN_WHISTLE]);
    regions[RegionCode::RYUMA]->addPathsBetween(regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE], items[ITEM_SUN_STONE]);
    regions[RegionCode::RYUMA]->addPathsBetween(regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]);
   	regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]->addPathsBetween(regions[RegionCode::THIEVES_HIDEOUT_POST_KEY], items[ITEM_KEY]); 
	regions[RegionCode::MERCATOR]->addPathsBetween(regions[RegionCode::MERCATOR_DUNGEON]);
    regions[RegionCode::MERCATOR]->addPathsBetween(regions[RegionCode::MERCATOR_CASINO], items[ITEM_CASINO_TICKET]);
	regions[RegionCode::MERCATOR]->addPathsBetween(regions[RegionCode::CRYPT]);
	regions[RegionCode::MERCATOR]->addPathsBetween(regions[RegionCode::MIR_TOWER_SECTOR]);
	regions[RegionCode::MERCATOR]->addPathsBetween(regions[RegionCode::MERCATOR_SPECIAL_SHOP], items[ITEM_BUYER_CARD]);
	regions[RegionCode::MIR_TOWER_SECTOR]->addPathsBetween(regions[RegionCode::MIR_TOWER_PRE_GARLIC], items[ITEM_ARMLET]);
    regions[RegionCode::MIR_TOWER_SECTOR]->addPathsBetween(regions[RegionCode::TWINKLE_VILLAGE]);
    regions[RegionCode::MIR_TOWER_SECTOR]->addPathsBetween(regions[RegionCode::MIR_TOWER_SECTOR_BEHIND_TREES], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::MIR_TOWER_PRE_GARLIC]->addPathsBetween(regions[RegionCode::MIR_TOWER_POST_GARLIC], { items[ITEM_GARLIC], items[ITEM_GARLIC] });
	regions[RegionCode::VERLA_SECTOR]->addPathsBetween(regions[RegionCode::VERLA_MINES]);
    regions[RegionCode::VERLA_SECTOR]->addPathsBetween(regions[RegionCode::VERLA]);
	regions[RegionCode::VERLA_MINES]->addPathsBetween(regions[RegionCode::ROUTE_VERLA_DESTEL]);
	regions[RegionCode::ROUTE_VERLA_DESTEL]->addPathsBetween(regions[RegionCode::DESTEL]);
	regions[RegionCode::DESTEL]->addPathsBetween(regions[RegionCode::ROUTE_AFTER_DESTEL]);
	regions[RegionCode::DESTEL]->addPathsBetween(regions[RegionCode::DESTEL_WELL]);
	regions[RegionCode::DESTEL_WELL]->addPathsBetween(regions[RegionCode::ROUTE_LAKE_SHRINE]);
	regions[RegionCode::ROUTE_LAKE_SHRINE]->addPathsBetween(regions[RegionCode::LAKE_SHRINE]);
    regions[RegionCode::GREENMAZE_PRE_WHISTLE]->addPathsBetween(regions[RegionCode::GREENMAZE_POST_WHISTLE], items[ITEM_EINSTEIN_WHISTLE]);
	regions[RegionCode::GREENMAZE_PRE_WHISTLE]->addPathsBetween(regions[RegionCode::MOUNTAINOUS_AREA], items[ITEM_AXE_MAGIC]);
	regions[RegionCode::MOUNTAINOUS_AREA]->addPathsBetween(regions[RegionCode::KN_CAVE], items[ITEM_GOLA_EYE], 2);

	regions[RegionCode::MERCATOR]->addPathTo(regions[RegionCode::GREENMAZE_PRE_WHISTLE], items[ITEM_KEY], 2);
    regions[RegionCode::GREENMAZE_PRE_WHISTLE]->addPathTo(regions[RegionCode::MERCATOR]);
    regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE]->addPathTo(regions[RegionCode::VERLA_SECTOR], items[ITEM_SAFETY_PASS]);
	regions[RegionCode::VERLA_SECTOR]->addPathTo(regions[RegionCode::MIR_TOWER_SECTOR]);

    if(_options.handleGhostJumpingInLogic())
        regions[RegionCode::MOUNTAINOUS_AREA]->addPathsBetween(regions[RegionCode::ROUTE_LAKE_SHRINE], items[ITEM_AXE_MAGIC]);
    else
        regions[RegionCode::MOUNTAINOUS_AREA]->addPathTo(regions[RegionCode::ROUTE_LAKE_SHRINE], items[ITEM_AXE_MAGIC]);

    regions[RegionCode::GREENMAZE_POST_WHISTLE]->addPathTo(regions[RegionCode::ROUTE_MASSAN_GUMI]);
    regions[RegionCode::LAKE_SHRINE]->addPathTo(regions[RegionCode::MIR_TOWER_SECTOR]);
	WorldPath* kazaltTeleporter = regions[RegionCode::KN_CAVE]->addPathTo(regions[RegionCode::KAZALT], requiredJewels);
    kazaltTeleporter->addNonRequiredItemPlacedWhenCrossing(items[ITEM_LITHOGRAPH]);
    regions[RegionCode::KAZALT]->addPathTo(regions[RegionCode::KN_CAVE]);
    regions[RegionCode::KN_CAVE]->addPathTo(regions[RegionCode::MERCATOR]);

	regions[RegionCode::KAZALT]->addPathsBetween(regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]);
	regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]->addPathsBetween(regions[RegionCode::KN_LABYRINTH_POST_SPIKES], items[ITEM_SPIKE_BOOTS]);
	regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addPathsBetween(regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR], { items[ITEM_LOGS], items[ITEM_LOGS] });
	regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addPathsBetween(regions[RegionCode::KN_PALACE]);
	regions[RegionCode::KN_PALACE]->addPathsBetween(regions[RegionCode::ENDGAME], { items[ITEM_GOLA_FANG], items[ITEM_GOLA_HORN],  items[ITEM_GOLA_NAIL] });
}

void World::initRegionHints()
{
    regions[RegionCode::MASSAN]->addHint("in a village");
    regions[RegionCode::MASSAN_AFTER_SWAMP_SHRINE]->addHint("in a village");
    regions[RegionCode::GUMI]->addHint("in a village");
    regions[RegionCode::GUMI_AFTER_SWAMP_SHRINE]->addHint("in a village");
    regions[RegionCode::DESTEL]->addHint("in a village");
    regions[RegionCode::TWINKLE_VILLAGE]->addHint("in a village");

    regions[RegionCode::RYUMA]->addHint("in a town");
    regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE]->addHint("in a town");
    regions[RegionCode::MERCATOR]->addHint("in a town");
    regions[RegionCode::VERLA]->addHint("in a town");

    regions[RegionCode::ROUTE_MASSAN_GUMI]->addHint("on a route");
    regions[RegionCode::ROUTE_GUMI_RYUMA]->addHint("on a route");
    regions[RegionCode::MIR_TOWER_SECTOR]->addHint("on a route");
    regions[RegionCode::MIR_TOWER_SECTOR_BEHIND_TREES]->addHint("on a route");
    regions[RegionCode::VERLA_SECTOR]->addHint("on a route");
    regions[RegionCode::ROUTE_VERLA_DESTEL]->addHint("on a route");
    regions[RegionCode::ROUTE_AFTER_DESTEL]->addHint("on a route");
    regions[RegionCode::ROUTE_LAKE_SHRINE]->addHint("on a route");

    regions[RegionCode::WATERFALL_SHRINE]->addHint("in a shrine");
    regions[RegionCode::SWAMP_SHRINE]->addHint("in a shrine");
    regions[RegionCode::LAKE_SHRINE]->addHint("in a shrine");

    regions[RegionCode::WATERFALL_SHRINE]->addHint("close to a waterfall");
    regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]->addHint("close to a waterfall");
    regions[RegionCode::THIEVES_HIDEOUT_POST_KEY]->addHint("close to a waterfall");
    regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR]->addHint("close to a waterfall");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_DAHL]->addHint("close to a waterfall");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_GOLDS]->addHint("close to a waterfall");
    itemSources[ItemSourceCode::CHEST_GREENMAZE_WATERFALL_CAVE_LIFESTOCK]->addHint("close to a waterfall");

    regions[RegionCode::GREENMAZE_PRE_WHISTLE]->addHint("among the trees");
    regions[RegionCode::GREENMAZE_POST_WHISTLE]->addHint("among the trees");
    regions[RegionCode::TIBOR]->addHint("among the trees");
    regions[RegionCode::MIR_TOWER_SECTOR_BEHIND_TREES]->addHint("among the trees");
    itemSources[ItemSourceCode::GROUND_LOGS_1]->addHint("among the trees");
    itemSources[ItemSourceCode::GROUND_LOGS_2]->addHint("among the trees");

    regions[RegionCode::SWAMP_SHRINE]->addHint("near a swamp");
    regions[RegionCode::WITCH_HELGA_HUT]->addHint("near a swamp");
    itemSources[ItemSourceCode::CHEST_ROUTE_MASSAN_GUMI_PROMONTORY]->addHint("near a swamp");

    regions[RegionCode::ROUTE_AFTER_DESTEL]->addHint("near a lake");
    regions[RegionCode::ROUTE_LAKE_SHRINE]->addHint("near a lake");
    regions[RegionCode::LAKE_SHRINE]->addHint("near a lake");
    
    regions[RegionCode::MASSAN]->addHint("in a region inhabited by bears");
    regions[RegionCode::GUMI]->addHint("in a region inhabited by bears");
    regions[RegionCode::ROUTE_MASSAN_GUMI]->addHint("in a region inhabited by bears");
    regions[RegionCode::ROUTE_GUMI_RYUMA]->addHint("in a region inhabited by bears");
    regions[RegionCode::WATERFALL_SHRINE]->addHint("in a region inhabited by bears");
    regions[RegionCode::SWAMP_SHRINE]->addHint("in a region inhabited by bears");

    regions[RegionCode::DESTEL]->addHint("in Destel region");
    regions[RegionCode::DESTEL_WELL]->addHint("in Destel region");
    regions[RegionCode::ROUTE_VERLA_DESTEL]->addHint("in Destel region");
    regions[RegionCode::ROUTE_AFTER_DESTEL]->addHint("in Destel region");

    regions[RegionCode::CRYPT]->addHint("hidden in the depths of Mercator");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_KEY]->addHint("hidden in the depths of Mercator");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_LIFESTOCK]->addHint("hidden in the depths of Mercator");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_LEFT]->addHint("hidden in the depths of Mercator");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_UNDERGROUND_DUAL_EKEEKE_RIGHT]->addHint("hidden in the depths of Mercator");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO]->addHint("hidden in the depths of Mercator");

    regions[RegionCode::MIR_TOWER_PRE_GARLIC]->addHint("inside a tower");
    regions[RegionCode::MIR_TOWER_POST_GARLIC]->addHint("inside a tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_LEFT]->addHint("inside a tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_DUAL_EKEEKE_RIGHT]->addHint("inside a tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_DUNGEON_TOWER_LIFESTOCK]->addHint("inside a tower");
    itemSources[ItemSourceCode::CHEST_MERCATOR_ARTHUR_KEY]->addHint("inside a tower");

    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN]->addHint("in a small cave");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_VISIBLE]->addHint("in a small cave");
    itemSources[ItemSourceCode::GROUND_MOON_STONE]->addHint("in a small cave");

    regions[RegionCode::MASSAN_CAVE]->addHint("in a large cave");
    regions[RegionCode::KN_CAVE]->addHint("in a large cave");
    regions[RegionCode::DESTEL_WELL]->addHint("in a large cave");
    regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]->addHint("in a large cave");
    regions[RegionCode::THIEVES_HIDEOUT_POST_KEY]->addHint("in a large cave");

    regions[RegionCode::KAZALT]->addHint("in King Nole's domain");
    regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]->addHint("in King Nole's domain");
    regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addHint("in King Nole's domain");
    regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR]->addHint("in King Nole's domain");
    regions[RegionCode::KN_PALACE]->addHint("in King Nole's domain");

    itemSources[ItemSourceCode::CHEST_MASSAN_DOG_STATUE]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MERCATOR_CASINO]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_VERLA_SECTOR_BEHIND_CABIN]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_VERLA_WELL]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_ROUTE_VERLA_DESTEL_HIDDEN_LIFESTOCK]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_ROUTE_AFTER_DESTEL_HIDDEN_LIFESTOCK]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_BELOW_ROCKY_ARCH]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GOLDEN_STATUE]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_HIDDEN_GAIA_STATUE]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN]->addHint("in a well-hidden chest");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_HIDDEN_EKEEKE]->addHint("in a well-hidden chest");

    itemSources[ItemSourceCode::GROUND_HEALING_BOOTS]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_MIRO_REWARD]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_REWARD]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_SPINNER_KEY]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::GROUND_HYPER_BREAST]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_KN_LABYRINTH_B3_FIREDEMON_REWARD]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::NPC_ZAK_GOLA_EYE]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_VERLA_MINES_SLASHER_KEY]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_LEFT]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_MIDDLE]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_LAKE_SHRINE_B3_REWARD_RIGHT]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_PURPLE_JEWEL]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LEFT_EKEEKE]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_RIGHT_EKEEKE]->addHint("kept by a threatening guardian");
    itemSources[ItemSourceCode::CHEST_MIR_TOWER_REWARD_LIFESTOCK]->addHint("kept by a threatening guardian");

    regions[RegionCode::MASSAN]->addHint("in the village of Massan");
    regions[RegionCode::MASSAN_AFTER_SWAMP_SHRINE]->addHint("in the village of Massan");
    regions[RegionCode::GUMI]->addHint("in the village of Gumi");
    regions[RegionCode::GUMI_AFTER_SWAMP_SHRINE]->addHint("in the village of Gumi");
    regions[RegionCode::DESTEL]->addHint("in the village of Destel");
    regions[RegionCode::TWINKLE_VILLAGE]->addHint("in Twinkle village");
    regions[RegionCode::RYUMA]->addHint("in the town of Ryuma");
    regions[RegionCode::RYUMA_REPAIRED_LIGHTHOUSE]->addHint("in the town of Ryuma");
    regions[RegionCode::MERCATOR]->addHint("in the town of Mercator");
    regions[RegionCode::VERLA]->addHint("in the town of Verla");
    regions[RegionCode::ROUTE_MASSAN_GUMI]->addHint("between Massan and Gumi");
    regions[RegionCode::ROUTE_GUMI_RYUMA]->addHint("between Gumi and Ryuma");
    regions[RegionCode::MIR_TOWER_SECTOR]->addHint("nearby Mir Tower");
    regions[RegionCode::MIR_TOWER_SECTOR_BEHIND_TREES]->addHint("nearby Mir Tower");
    regions[RegionCode::VERLA_SECTOR]->addHint("nearby the town of Verla");
    regions[RegionCode::ROUTE_VERLA_DESTEL]->addHint("between Verla and Destel");
    regions[RegionCode::ROUTE_AFTER_DESTEL]->addHint("on the route to the lake after Destel");
    regions[RegionCode::ROUTE_LAKE_SHRINE]->addHint("on the mountainous path to Lake Shrine");
    regions[RegionCode::WATERFALL_SHRINE]->addHint("in Waterfall Shrine");
    regions[RegionCode::SWAMP_SHRINE]->addHint("in Swamp Shrine");
    regions[RegionCode::LAKE_SHRINE]->addHint("in Lake Shrine");
    regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]->addHint("in the Thieves' Hideout");
    regions[RegionCode::THIEVES_HIDEOUT_POST_KEY]->addHint("in the Thieves' Hideout");
    regions[RegionCode::GREENMAZE_PRE_WHISTLE]->addHint("in the infamous Greenmaze");
    regions[RegionCode::GREENMAZE_POST_WHISTLE]->addHint("in the infamous Greenmaze");
    regions[RegionCode::TIBOR]->addHint("inside the elder tree called Tibor");
    regions[RegionCode::WITCH_HELGA_HUT]->addHint("in the hut of a witch called Helga");
    regions[RegionCode::DESTEL_WELL]->addHint("in Destel Well");
    regions[RegionCode::CRYPT]->addHint("in Mercator crypt");
    regions[RegionCode::MIR_TOWER_PRE_GARLIC]->addHint("in Mir Tower");
    regions[RegionCode::MIR_TOWER_POST_GARLIC]->addHint("in Mir Tower");
    regions[RegionCode::MASSAN_CAVE]->addHint("in Massan cave");
    regions[RegionCode::KN_CAVE]->addHint("in King Nole's cave");
    regions[RegionCode::VERLA_MINES]->addHint("in Verla Mines");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_HIDDEN]->addHint("in a cave in the mountains");
    itemSources[ItemSourceCode::CHEST_MOUNTAINOUS_AREA_CAVE_VISIBLE]->addHint("in a cave in the mountains");
    itemSources[ItemSourceCode::GROUND_MOON_STONE]->addHint("in a cave in the mountains");
    regions[RegionCode::MOUNTAINOUS_AREA]->addHint("in a mountainous area");
    regions[RegionCode::KAZALT]->addHint("in Kazalt");
    regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]->addHint("in King Nole's labyrinth");
    regions[RegionCode::KN_LABYRINTH_POST_SPIKES]->addHint("in King Nole's labyrinth");
    regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR]->addHint("in King Nole's labyrinth");
    regions[RegionCode::KN_PALACE]->addHint("in King Nole's palace");
}

void World::initHintSigns(bool fillDungeonSignsWithHints)
{
    hintSigns = {
        new HintSign(0x101, "Waterfall Shrine crossroad sign", regions[RegionCode::ROUTE_MASSAN_GUMI]),
        new HintSign(0x102, "Swamp Shrine crossroad sign", regions[RegionCode::ROUTE_MASSAN_GUMI]),
        new HintSign(0x103, "Tibor crossroad sign", regions[RegionCode::ROUTE_GUMI_RYUMA]),
        new HintSign(0x104, "Mir Tower sector crossroad sign", regions[RegionCode::MIR_TOWER_SECTOR]),
        new HintSign(0x134, "Mir Tower exterior sign", regions[RegionCode::MIR_TOWER_SECTOR]),
        new HintSign(0x143, "Verla crossroad sign", regions[RegionCode::VERLA_SECTOR]),
        new HintSign(0x142, "Destel crossroad sign", regions[RegionCode::ROUTE_VERLA_DESTEL]),
        new HintSign(0x141, "Lake Shrine / Mountainous crossroad sign", regions[RegionCode::ROUTE_LAKE_SHRINE]),
        new HintSign(0x140, "Greenmaze / Mountainous crossroad sign", regions[RegionCode::GREENMAZE_PRE_WHISTLE]),
        new HintSign(0x139, "Center of Greenmaze sign", regions[RegionCode::GREENMAZE_PRE_WHISTLE]),
        new HintSign(0x13A, "Greenmaze / Massan shortcut tunnel sign", regions[RegionCode::GREENMAZE_POST_WHISTLE]),
        new HintSign(0x144, "Volcano sign", regions[RegionCode::LAKE_SHRINE])
    };

    if (fillDungeonSignsWithHints)
    {
        hintSigns.push_back(new HintSign(0x0FE, "Thieves' Hideout entrance sign", regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]));
        hintSigns.push_back(new HintSign(0x0FF, "Thieves' Hideout second room sign", regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]));
        hintSigns.push_back(new HintSign(0x100, "Thieves' Hideout boss path sign", regions[RegionCode::THIEVES_HIDEOUT_POST_KEY]));
        hintSigns.push_back(new HintSign({0x12C,0x12D,0x12E}, "Mir Tower sign before bridge room", regions[RegionCode::MIR_TOWER_PRE_GARLIC]));
        hintSigns.push_back(new HintSign(0x12F, "Mir Tower bridge room sign", regions[RegionCode::MIR_TOWER_PRE_GARLIC]));
        hintSigns.push_back(new HintSign({0x130,0x131}, "Mir Tower library sign", regions[RegionCode::MIR_TOWER_POST_GARLIC]));
        hintSigns.push_back(new HintSign({0x132,0x133}, "Mir Tower sign before library", regions[RegionCode::MIR_TOWER_POST_GARLIC]));
    }
}

void World::initDarkRooms()
{
    regions[RegionCode::WATERFALL_SHRINE]->setDarkRooms(0xAE, 0xB6);

    regions[RegionCode::SWAMP_SHRINE]->setDarkRooms({
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xC, 0xD, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1B, 0x1E
    });
    
    regions[RegionCode::THIEVES_HIDEOUT_PRE_KEY]->setDarkRooms({
        0xB9, 0xBA, 0xBC, 0xBD, 0xBE, 0xBF, 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 
        0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0, 0xD2, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE
    });

    regions[RegionCode::MASSAN_CAVE]->setDarkRooms(0x323, 0x327);

    regions[RegionCode::VERLA_MINES]->setDarkRooms({
        0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xED, 0xEF, 0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF6, 0xF7, 0xF8, 0xFA, 0xFD, 0xFE, 0xFF, 
        0x100, 0x102, 0x103, 0x10A, 0x10C, 0x10D
    });

    regions[RegionCode::MERCATOR_DUNGEON]->setDarkRooms({
        0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x4C, 0x50, 0x51, 0x52, 0x5B, 0x5C
    });

    regions[RegionCode::MIR_TOWER_PRE_GARLIC]->setDarkRooms(0x2EE, 0x310);

    regions[RegionCode::CRYPT]->setDarkRooms(0x286, 0x293);

    regions[RegionCode::DESTEL_WELL]->setDarkRooms(0x10E, 0x122);

    regions[RegionCode::LAKE_SHRINE]->setDarkRooms(0x123, 0x162);

    regions[RegionCode::KN_CAVE]->setDarkRooms({
        0x91, 0x93, 0x96, 0x98, 0x9A, 0x9B, 0x9C, 0x9E, 0xA0, 0xA1, 0xA2, 0xA4, 0xA6, 0xAA, 0xAB, 0xAC
    });

    regions[RegionCode::KN_LABYRINTH_PRE_SPIKES]->setDarkRooms({
        0x163, 0x164, 0x165, 0x166, 0x167, 0x168, 0x169, 0x16B, 0x16C, 0x16D, 0x16E, 0x16F, 0x170, 0x171, 
        0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x178, 0x179, 0x17A, 0x17B, 0x17C, 0x17D, 0x17E, 0x17F, 
        0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188, 0x189, 0x18A, 0x18B, 0x18C, 0x18D, 
        0x18E, 0x195, 0x196, 0x198, 0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A1, 0x1A2, 
        0x1A3, 0x1A4, 0x1A6, 0x1A7
    });
    
    regions[RegionCode::KN_PALACE]->setDarkRooms(0x73, 0x8A);

    regions[RegionCode::WITCH_HELGA_HUT]->setDarkRooms(0x311, 0x322);

    regions[RegionCode::TIBOR]->setDarkRooms(0x328, 0x32F);
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

Json World::toJSON() const
{
	Json json;

    // Export dark region
    json["spawnRegion"] = regions.at(getSpawnLocationRegion(spawnLocation))->getName();
    json["darkRegion"] = darkenedRegion->getName();

    // Export hints
    for(HintSign* sign : hintSigns)
		json["hints"][sign->getDescription()] = sign->getText();
    json["hints"]["Lithograph"] = jewelHints;
    json["hints"]["Oracle Stone"] = oracleStoneHint;
    json["hints"]["King Nole's Cave sign"] = whereIsLithographHint;
    json["hints"]["Mercator fortune teller"] = fortuneTellerHint;

    // Export item sources
	for(auto& it : regions)
	{
		const WorldRegion& region = *it.second;
		for(ItemSource* source : region.getItemSources())
		{
            Item* item = items.at(source->getItemID());
			json["itemSources"][region.getName()][source->getName()] = item->getName();
		}
	}

    return json;
}

void World::parseJSON(const Json& json)
{
    ////////// Item Sources ///////////////////////////////////////////
    const Json& itemSourcesJson = json.at("itemSources");
	for(auto& it : regions)
	{
		const WorldRegion& region = *it.second;
        if(region.getItemSources().empty())
            continue;

        if(itemSourcesJson.contains(region.getName()))
        {
            const Json& regionJson = itemSourcesJson.at(region.getName());
            for(ItemSource* source : region.getItemSources())
            {
                if(regionJson.contains(source->getName()))
                {
                    std::string itemName = regionJson.at(source->getName());
                    Item* item = this->parseItemFromName(itemName);
                    if(item)
                    {
                        source->setItem(item);
                    }
                    else
                    {
                        std::stringstream msg;
                        msg << "Item name '" << itemName << "' is invalid in plando JSON.";
                        throw RandomizerException(msg.str());
                    }
                }
                else
                {
                    std::stringstream msg;
                    msg << "Item source '" << source->getName() << "' is missing from plando JSON.";
                    throw RandomizerException(msg.str());
                }
            }
        }
        else
        {
            std::stringstream msg;
            msg << "Region '" << region.getName() << "' is missing from plando JSON.";
            throw RandomizerException(msg.str());
        }
	}

    ////////// Hints ///////////////////////////////////////////
    const Json& hintsJson = json.at("hints");
   
    if(hintsJson.contains("Lithograph"))
    {
        if(hintsJson.at("Lithograph").is_array())
        {
            for(const std::string& jewelHint : hintsJson.at("Lithograph"))
                jewelHints.push_back(jewelHint);
        }
        else throw RandomizerException("Lithograph hint must be an array of strings in plando JSON.");
    }

    if(hintsJson.contains("Oracle Stone"))
        oracleStoneHint = hintsJson.at("Oracle Stone");
    else
        throw RandomizerException("Oracle Stone hint is missing from plando JSON.");

    if(hintsJson.contains("King Nole's Cave sign"))
        whereIsLithographHint = hintsJson.at("King Nole's Cave sign");
    else
        throw RandomizerException("King Nole's Cave sign hint is missing from plando JSON.");

    if(hintsJson.contains("Mercator fortune teller"))
        fortuneTellerHint = hintsJson.at("Mercator fortune teller");
    else
        throw RandomizerException("Mercator fortune teller hint is missing from plando JSON.");

    for(HintSign* sign : hintSigns)
    {
        if(hintsJson.contains(sign->getDescription()))
            sign->setText(hintsJson.at(sign->getDescription()));
        else
        {
            std::stringstream msg;
            msg << "Sign hint '" << sign->getDescription() << "' is missing from plando JSON.";
            throw RandomizerException(msg.str());
        }
    }

    ////////// Miscellaneous ///////////////////////////////////////////
    if(json.contains("darkRegion"))
    {
        std::string darkRegionName = json.at("darkRegion");
        darkenedRegion = this->getRegionByName(darkRegionName);
        if(!darkenedRegion)
        {
            std::stringstream msg;
            msg << "Darkened region name '" << darkRegionName << "' is invalid in plando JSON.";
            throw RandomizerException(msg.str());
        }
    }
    else throw RandomizerException("Darkened region is missing from plando JSON.");
}

Item* World::parseItemFromName(const std::string& itemName)
{
    Item* item = this->getItemByName(itemName);
    if(item)
        return item;

    // If item is formatted as "X golds", parse X value and create the matching gold stack item
    if(Tools::endsWith(itemName, "golds"))
    {
        size_t spaceIndex = itemName.find_first_of(' ');
        if(spaceIndex == std::string::npos)
            return nullptr;

        std::string numberPart = itemName.substr(0, spaceIndex);
        
        try {
            uint8_t goldValue = static_cast<uint8_t>(std::strtol(numberPart.c_str(), nullptr, 10));
            return this->addGoldItem(goldValue);
        }
        catch(std::exception&) {}
    }

    return nullptr;
}

std::vector<Item*> World::findSmallestInventoryToReachRegion(WorldRegion* endRegion) const
{
    WorldRegion* spawnRegion = regions.at(getSpawnLocationRegion(spawnLocation));
    WorldSolver solver(spawnRegion, endRegion);
    solver.tryToSolve();
    std::vector<Item*> inventory = solver.getInventory();

    std::vector<Item*> minimalInventory;

    UnsortedSet<Item*> forbiddenItems;

    for(Item* item : inventory)
    {  
        UnsortedSet<Item*> forbiddenItemsPlusOne = forbiddenItems;
        forbiddenItemsPlusOne.insert(item);
        
        WorldSolver solver2(spawnRegion, endRegion);
        solver2.forbidItems(forbiddenItemsPlusOne);
        if(solver2.tryToSolve())
        {
            // Item can be freely removed: keep it removed for further solves
            forbiddenItems = forbiddenItemsPlusOne;
        }
        else
        {
            // Item cannot be removed: it means it's required
            minimalInventory.push_back(item);
        }
    }
 
    return minimalInventory;
}

bool World::isMacroRegionAvoidable(WorldMacroRegion* macroRegion) const
{
    WorldRegion* spawnRegion = regions.at(getSpawnLocationRegion(spawnLocation));
    WorldRegion* endRegion = regions.at(RegionCode::ENDGAME);
    WorldSolver solver(spawnRegion, endRegion);
    solver.forbidTakingItemsFromRegions(macroRegion->getRegions());
    
    return solver.tryToSolve();
}
