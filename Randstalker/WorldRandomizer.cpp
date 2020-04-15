#include "WorldRandomizer.h"
#include "Tools.h"
#include "GameText.h"
#include <algorithm>
#include <set>
#include <sstream>

WorldRandomizer::WorldRandomizer(World& world, const RandomizerOptions& options) :
	_world				(world),
	_options			(options),
	_rng				()
{
	if (!options.getDebugLogPath().empty())
		_debugLog.open(options.getDebugLogPath());
}

WorldRandomizer::~WorldRandomizer() 
{
	_debugLog.close();
}

void WorldRandomizer::randomize()
{
	uint32_t rngSeed = _options.getSeed();

	// 1st pass: randomizations happening BEFORE randomizing items
	_rng.seed(rngSeed);
	this->randomizeGoldValues();
	this->randomizeDarkRooms();

	// 2nd pass: randomizing items
	_rng.seed(rngSeed);
	this->randomizeItems();
	this->analyzeStrictlyRequiredKeyItems();
	
	// 3rd pass: randomizations happening AFTER randomizing items
	_rng.seed(rngSeed);
	this->randomizeSpawnLocation();
	this->randomizeHints();

	if(_options.shuffleTiborTrees())
		this->randomizeTiborTrees();
}



///////////////////////////////////////////////////////////////////////////////////////
///		FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeGoldValues()
{
	constexpr uint16_t averageGoldPerChest = 35;
	constexpr double maxFactorOfTotalGoldValue = 0.16;

	uint16_t totalGoldValue = averageGoldPerChest * GOLD_SOURCES_COUNT;

	for (uint8_t i = 0; i < GOLD_SOURCES_COUNT ; ++i)
	{
		uint16_t goldValue;

		if (i < GOLD_SOURCES_COUNT - 1)
		{
			uint32_t randomNumber = _rng();
			double proportion = (double) randomNumber / (double) UINT32_MAX;
			double factor = (proportion * maxFactorOfTotalGoldValue);

			goldValue = (uint16_t)((double)totalGoldValue * factor);
		}
		else
		{
			goldValue = totalGoldValue;
		}

		if (goldValue == 0)
			goldValue = 1;
		else if (goldValue > 255)
			goldValue = 255;

		totalGoldValue -= goldValue;

		std::ostringstream goldName;
		goldName << (uint32_t)goldValue << " golds";

		_world.addItem(new Item(ITEM_GOLDS_START + i, goldName.str(), (uint8_t)goldValue, false));
	}
}

void WorldRandomizer::randomizeDarkRooms()
{
	std::vector<WorldRegion*> darkenableRegions;
	for (auto& [key, region] : _world.regions)
	{
		if (!region->getDarkRooms().empty())
			darkenableRegions.push_back(region);
	}

	Tools::shuffle(darkenableRegions, _rng);
	_world.darkenedRegion = darkenableRegions[0];

	const std::vector<WorldPath*>& ingoingPaths = _world.darkenedRegion->getIngoingPaths();
	for (WorldPath* path : ingoingPaths)
		path->addRequiredItem(_world.items[ITEM_LANTERN]);
}



///////////////////////////////////////////////////////////////////////////////////////
///		SECOND PASS RANDOMIZATIONS (items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeItems()
{
	constexpr double FILLING_RATE = 0.21;

	_regionsToExplore = { _world.regions[RegionCode::MASSAN] };
	_exploredRegions.clear();		// Regions already processed by the exploration algorithm
	_itemSourcesToFill.clear();		// Reachable empty item sources which must be filled with a random item
	_playerInventory.clear();		// The current contents of player inventory at the given time in the exploration algorithm
	_pendingItemSources.clear();	// Unreachable item sources locked behind a key item which is not yet owned by the player
	_pendingPaths.clear();			// Paths leading to potentially unexplored regions, locked behind a key item which is not yet owned by the player

	this->initFillerItems();

	_debugLog << "\n-------------------------------------------------\n";
	_debugLog << "Step #0 (placing priority items)\n";
	this->placePriorityItems();

	uint32_t stepCount = 1;
	while (!_pendingPaths.empty() || !_regionsToExplore.empty())
	{
		_debugLog << "\n-------------------------------------------------\n";
		_debugLog << "Step #" << stepCount++ << "\n";

		// If there is going to be newly explored regions in this step, fill a fraction of already available sources with filler items
		if (!_regionsToExplore.empty() && !_itemSourcesToFill.empty())
		{
			size_t sourcesToFillCount = (size_t)(_itemSourcesToFill.size() * FILLING_RATE);
			this->placeFillerItemsPhase(sourcesToFillCount);
		}

		// Explore not yet explored regions, listing all item sources and paths for further exploration and processing
		this->explorationPhase();

		// Try unlocking paths and item sources with newly discovered items in pre-filled item sources (useful for plando)
		this->unlockPhase();

		Tools::shuffle(_itemSourcesToFill, _rng);

		// Place a key item to unlock access to new regions & item sources
		this->placeKeyItemPhase();

		// Try unlocking paths and item sources with the newly acquired key item
		this->unlockPhase();
	}

	// Place the remaining filler items, and put Ekeeke in the last empty sources
	this->placeFillerItemsPhase();
	for (ItemSource* itemSource : _itemSourcesToFill)
		itemSource->setItem(_world.items[ITEM_EKEEKE]);

	if (!_fillerItems.empty())
	{
		_debugLog << "\n-------------------------------\n";
		_debugLog << "\tUnplaced items" << "\n\n";

		for (Item* item : _fillerItems)
			_debugLog << "- [" << item->getName() << "]\n";
	}

	_debugLog << "\n-------------------------------------------------\n";
	_debugLog << "End of generation\n";
	_debugLog << "\t - " << _itemSourcesToFill.size() << " remaining sources to fill\n";
	for(auto source : _itemSourcesToFill)
		_debugLog << "\t\t - " << source->getName() << "\n";
	_debugLog << "\t - " << _pendingItemSources.size() << " pending item sources\n";
	_debugLog << "\t - " << _pendingPaths.size() << " pending paths\n";
}

void WorldRandomizer::initFillerItems()
{
	// TODO: Don't place already placed items in plandos

	_fillerItems = {
		_world.items[ITEM_GARLIC],			_world.items[ITEM_PAWN_TICKET],
		_world.items[ITEM_SHORT_CAKE],		_world.items[ITEM_SPELL_BOOK],
		_world.items[ITEM_BLUE_RIBBON],		_world.items[ITEM_ORACLE_STONE]
	};

	for (uint8_t i = 0; i < 80; ++i)
		_fillerItems.push_back(_world.items[ITEM_LIFESTOCK]);
	for (uint8_t i = 0; i < 55; ++i)
		_fillerItems.push_back(_world.items[ITEM_EKEEKE]);
	for (uint8_t i = 0; i < 16; ++i)
		_fillerItems.push_back(_world.items[ITEM_DAHL]);
	for (uint8_t i = 0; i < 12; ++i)
		_fillerItems.push_back(_world.items[ITEM_GAIA_STATUE]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_world.items[ITEM_GOLDEN_STATUE]);
	for (uint8_t i = 0; i < 10; ++i)
		_fillerItems.push_back(_world.items[ITEM_RESTORATION]);
	for (uint8_t i = 0; i < 12; ++i)
		_fillerItems.push_back(_world.items[ITEM_DETOX_GRASS]);
	for (uint8_t i = 0; i < 7; ++i)
		_fillerItems.push_back(_world.items[ITEM_MIND_REPAIR]);
	for (uint8_t i = 0; i < 7; ++i)
		_fillerItems.push_back(_world.items[ITEM_ANTI_PARALYZE]);

	for (uint8_t i = 0; i < GOLD_SOURCES_COUNT; ++i)
		_fillerItems.push_back(_world.items[ITEM_GOLDS_START + i]);

	for (uint8_t i = 0; i < 4; ++i)
		_fillerItems.push_back(_world.items[ITEM_NONE]);

	Tools::shuffle(_fillerItems, _rng);
}

void WorldRandomizer::placePriorityItems()
{
	// TODO: Don't place already placed items in plandos

	// Priority items are filler items which are always placed first in the randomization, no matter what
	std::vector<Item*> priorityItems = {
		_world.items[ITEM_MAGIC_SWORD],			_world.items[ITEM_THUNDER_SWORD],
		_world.items[ITEM_ICE_SWORD],			_world.items[ITEM_GAIA_SWORD],
		_world.items[ITEM_STEEL_BREAST],		_world.items[ITEM_CHROME_BREAST],
		_world.items[ITEM_SHELL_BREAST],		_world.items[ITEM_HYPER_BREAST],
		_world.items[ITEM_HEALING_BOOTS],		_world.items[ITEM_IRON_BOOTS],
		_world.items[ITEM_FIREPROOF_BOOTS],		_world.items[ITEM_MARS_STONE],
		_world.items[ITEM_MOON_STONE],			_world.items[ITEM_SATURN_STONE],
		_world.items[ITEM_VENUS_STONE],			_world.items[ITEM_DEATH_STATUE],
		_world.items[ITEM_BELL],				_world.items[ITEM_STATUE_JYPTA]
	};
	Tools::shuffle(priorityItems, _rng);

	std::vector<ItemSource*> allEmptyItemSources;
	for (auto& [key, source] : _world.itemSources)
		if(!source->getItem())
			allEmptyItemSources.push_back(source);
	Tools::shuffle(allEmptyItemSources, _rng);

	for (Item* itemToPlace : priorityItems)
	{
		for (ItemSource* source : allEmptyItemSources)
		{
			if (!source->getItem() && source->isItemCompatible(itemToPlace))
			{
				source->setItem(itemToPlace);
				_debugLog << "\t > Placing priority item [" << itemToPlace->getName() << "] in \"" << source->getName() << "\"\n";
				break;
			}
		}
	}
}

void WorldRandomizer::placeFillerItemsPhase(size_t count)
{
	if (!count)
		count = _itemSourcesToFill.size();

	_debugLog << "\t > Filling " << count << " item sources with filler items...\n";

	for (size_t i=0 ; i<count ; ++i)
	{
		ItemSource* itemSource = _itemSourcesToFill[0];
		
		for (size_t j=0 ; j < _fillerItems.size(); ++j)
		{
			Item* fillerItem = _fillerItems[j];
			if (itemSource->isItemCompatible(fillerItem))
			{
				itemSource->setItem(fillerItem);
				_debugLog << "\t\t - Filling \"" << itemSource->getName() << "\" with [" << fillerItem->getName() << "]\n";
				_fillerItems.erase(_fillerItems.begin() + j);
				_itemSourcesToFill.erase(_itemSourcesToFill.begin());
				break;
			}
		}
	}
}

void WorldRandomizer::explorationPhase()
{
	while (!_regionsToExplore.empty())
	{
		// Take and erase first region from regions to explore, add it to explored regions set.
		WorldRegion* exploredRegion = *_regionsToExplore.begin();
		_regionsToExplore.erase(exploredRegion);
		_exploredRegions.insert(exploredRegion);
		_debugLog << "\t > Exploring region \"" << exploredRegion->getName() << "\"...\n";

		// List item sources to fill from this region.
		const std::vector<ItemSource*> unrestrictedItemSources = exploredRegion->getUnrestrictedItemSources();
		for (ItemSource* itemSource : unrestrictedItemSources)
		{
			// Non-empty item sources populate player inventory (useful for plandos)
			if (itemSource->getItem())
				_playerInventory.insert(itemSource->getItem());
			else
				_itemSourcesToFill.push_back(itemSource);
		}
		// Add conditionnal item sources to a pending list.
		const std::map<ItemSource*, Item*>& restrictedItemSources = exploredRegion->getRestrictedItemSources();
		for (const auto& [itemSource, item] : restrictedItemSources)
			_pendingItemSources[itemSource] = item;

		// List outgoing paths
		for (WorldPath* outgoingPath : exploredRegion->getOutgoingPaths())
		{
			if (outgoingPath->canBeCrossedWithInventory(_playerInventory))
			{
				// For crossable paths, add destinations in regions to explore if not already explored / to process
				WorldRegion* destination = outgoingPath->getDestination();
				if (!_regionsToExplore.count(destination) && !_exploredRegions.count(destination))
					_regionsToExplore.insert(destination);
			}
			else
			{
				// For uncrossable blocked paths, add them to a pending list
				_pendingPaths.push_back(outgoingPath);
			}
		}
	}
}

void WorldRandomizer::placeKeyItemPhase()
{
	if (_pendingPaths.empty())
		return;

	// List all unowned key items, and pick a random one among them
	std::vector<Item*> unownedKeyItems;
	for (WorldPath* pendingPath : _pendingPaths)
	{
		for (Item* requiredItem : pendingPath->getRequiredItems())
		{
			if (!_playerInventory.count(requiredItem))
			{
				for(uint16_t i=0 ; i<pendingPath->getRandomWeight() ; ++i)
					unownedKeyItems.push_back(requiredItem);
			}
		}
	}

	Tools::shuffle(unownedKeyItems, _rng);
	Item* keyItemToPlace = unownedKeyItems[0];

	// Find a random item source capable of carrying the item
	ItemSource* randomItemSource = nullptr;
	for (uint32_t i = 0; i < _itemSourcesToFill.size(); ++i)
	{
		if (_itemSourcesToFill[i]->isItemCompatible(keyItemToPlace))
		{
			randomItemSource = _itemSourcesToFill[i];
			_itemSourcesToFill.erase(_itemSourcesToFill.begin() + i);
			break;
		}
	}
	if (!randomItemSource)
		throw NoAppropriateItemSourceException();

	// Place the key item in the appropriate source, and also add it to player inventory
	_debugLog << "\t > Key item is [" << keyItemToPlace->getName() << "], putting it in \"" << randomItemSource->getName() << "\"\n";
	randomItemSource->setItem(keyItemToPlace);
	_playerInventory.insert(keyItemToPlace);
}

void WorldRandomizer::unlockPhase()
{
	// Look for unlockable paths...
	for (size_t i=0 ; i < _pendingPaths.size() ; ++i)
	{
		WorldPath* pendingPath = _pendingPaths[i];

		if (pendingPath->canBeCrossedWithInventory(_playerInventory))
		{
			// Path is now unlocked, add destination to regions to explore if it has not yet been explored
			WorldRegion* destination = pendingPath->getDestination();
			if (!_regionsToExplore.count(destination) && !_exploredRegions.count(destination))
				_regionsToExplore.insert(destination);

			// Remove path from pending paths
			_pendingPaths.erase(_pendingPaths.begin()+i);
			--i;
		}
	}

	// Look for unlockable item sources...
	for (auto it = _pendingItemSources.begin(); it != _pendingItemSources.end(); )
	{
		ItemSource* itemSource = it->first;
		Item* requiredItem = it->second;

		if (_playerInventory.count(requiredItem))
		{
			// Item source is now unlocked, add it to the "item sources to fill" list if it's empty or take the pre-filled item
			if (itemSource->getItem())
				_playerInventory.insert(itemSource->getItem());
			else
				_itemSourcesToFill.push_back(itemSource);

			// Remove it from the pending list
			++it;
			_pendingItemSources.erase(itemSource);
		}
		else ++it;
	}
}

void WorldRandomizer::analyzeStrictlyRequiredKeyItems()
{
	const std::set<Item*> optionalItems = { _world.items[ITEM_LITHOGRAPH], _world.items[ITEM_LANTERN] };

	// We perform a backwards analysis here starting from endgame region and determining which key items are strictly needed to finish the seed
	std::set<WorldRegion*> regionsToExplore = { _world.regions[RegionCode::ENDGAME] };
	std::set<WorldRegion*> alreadyExploredRegions;
	std::set<Item*> itemsToLocate;
	
	while (!regionsToExplore.empty())
	{
		WorldRegion* exploredRegion = *regionsToExplore.begin();
		regionsToExplore.erase(exploredRegion);
		alreadyExploredRegions.insert(exploredRegion);

		const std::vector<WorldPath*>& pathsToRegion = exploredRegion->getIngoingPaths();
		for (WorldPath* path : pathsToRegion)
		{
			for (Item* neededItem : path->getRequiredItems())
				if (!optionalItems.count(neededItem))
					itemsToLocate.insert(neededItem);

			WorldRegion* originRegion = path->getOrigin();
			if (!alreadyExploredRegions.count(originRegion))
				regionsToExplore.insert(originRegion);
		}

		while (regionsToExplore.empty() && !itemsToLocate.empty())
		{
			Item* keyItemToLocate = *itemsToLocate.begin();
			itemsToLocate.erase(keyItemToLocate);			
			if (!_strictlyNeededKeyItems.count(keyItemToLocate))
			{
				_strictlyNeededKeyItems.insert(keyItemToLocate);

				for (const auto& [code, source] : _world.itemSources)
				{
					if (source->getItem() == keyItemToLocate)
					{
						WorldRegion* region = source->getRegion();
						if (!alreadyExploredRegions.count(region))
							regionsToExplore.insert(region);
						break;
					}
				}
			}
		}
	}

	// Output required item list to debug log if we are in debug mode
	if (_debugLog)
	{
		_debugLog << "\n-------------------------------\n";
		_debugLog << "\tItems required to finish the seed" << "\n\n";
		for (Item* item : _strictlyNeededKeyItems)
			_debugLog << "\t- " << item->getName() << "\n";
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///		THIRD PASS RANDOMIZATIONS (after items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeSpawnLocation()
{
	if (_world.spawnLocation == SpawnLocation::RANDOM)
	{
		std::vector<SpawnLocation> possibleSpawnRegions = { SpawnLocation::MASSAN, SpawnLocation::GUMI, SpawnLocation::RYUMA };
		Tools::shuffle(possibleSpawnRegions, _rng);
		_world.spawnLocation = possibleSpawnRegions[0];
	}
}

void WorldRandomizer::randomizeHints()
{
	// =============== Lithograph hint ===============

	std::string redJewelHint = "Red Jewel is " + this->getRandomHintForItem(_world.items[ITEM_RED_JEWEL]) + ".";
	std::string purpleJewelHint = "Purple Jewel is " + this->getRandomHintForItem(_world.items[ITEM_PURPLE_JEWEL]) + ".";
	std::string completeLithographHint = redJewelHint + "\t\n" + purpleJewelHint;
	
	_world.lithographHint = GameText(completeLithographHint);

	// =============== Fortune Teller hint ===============

	std::vector<Item*> hintableItemsByFortuneTeller = { _world.items[ITEM_GOLA_EYE], _world.items[ITEM_GOLA_NAIL], _world.items[ITEM_GOLA_FANG], _world.items[ITEM_GOLA_HORN] };
	Tools::shuffle(hintableItemsByFortuneTeller, _rng);
	Item* fortuneHintedItem = *(hintableItemsByFortuneTeller.begin());

	std::string fortuneItemName;
	if (fortuneHintedItem == _world.items[ITEM_GOLA_EYE]) 
		fortuneItemName = "an eye";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_NAIL]) 
		fortuneItemName = "a nail";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_FANG]) 
		fortuneItemName = "a fang";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_HORN])
		fortuneItemName = "a horn";

	std::string fortuneItemHint = this->getRandomHintForItem(fortuneHintedItem);
	std::string completeFortuneHint = "I see " + fortuneItemName + " " + fortuneItemHint + ".";

	_world.ingameTexts[0x27CE4] = GameText("Fortune Teller: Hello dear, let me look what your future is made of.");
	_world.ingameTexts[0x27CE6] = GameText("I see... I see... " + completeFortuneHint);

	// =============== King Nole Cave "where is lithograph" hint sign ===============

	std::string whereIsLithograph = "The lithograph will help you finding the jewels. It is " + this->getRandomHintForItem(_world.items[ITEM_LITHOGRAPH]) + ".";
	_world.ingameTexts[0x27958] = whereIsLithograph;

	// =============== Sign hints ===============

	std::vector<std::string> signHintsVector;

	const std::map<std::string, std::vector<WorldRegion*>> macroRegions = {
		{ "the village of Massan",		{ _world.regions[RegionCode::MASSAN] } },
		{ "the cave near Massan",		{ _world.regions[RegionCode::MASSAN_CAVE] } },
		{ "the waterfall shrine",		{ _world.regions[RegionCode::WATERFALL_SHRINE] } },
		{ "the village of Gumi",		{ _world.regions[RegionCode::GUMI] } },
		{ "the swamp shrine",			{ _world.regions[RegionCode::SWAMP_SHRINE] } },
		{ "Tibor",						{ _world.regions[RegionCode::TIBOR] } },
		{ "the town of Ryuma",			{ _world.regions[RegionCode::RYUMA] } },
		{ "the thieves' hideout",		{ _world.regions[RegionCode::THIEVES_HIDEOUT] } },
		{ "witch Helga's hut",			{ _world.regions[RegionCode::WITCH_HELGA_HUT] } },
		{ "the town of Mercator",		{ _world.regions[RegionCode::MERCATOR], _world.regions[RegionCode::MERCATOR_SPECIAL_SHOP] } },
		{ "the crypt of Mercator",		{ _world.regions[RegionCode::CRYPT] } },
		{ "the dungeon of Mercator",	{ _world.regions[RegionCode::MERCATOR_DUNGEON] } },
		{ "Mir Tower",					{ _world.regions[RegionCode::MIR_TOWER_PRE_GARLIC], _world.regions[RegionCode::MIR_TOWER_POST_GARLIC] } },
		{ "Greenmaze",					{ _world.regions[RegionCode::GREENMAZE] } },
		{ "the town of Verla",			{ _world.regions[RegionCode::VERLA] } },
		{ "Verla mine",					{ _world.regions[RegionCode::VERLA_MINES] } },
		{ "the village of Destel",		{ _world.regions[RegionCode::DESTEL] } },
		{ "Destel well",				{ _world.regions[RegionCode::DESTEL_WELL] } },
		{ "the lake shrine",			{ _world.regions[RegionCode::LAKE_SHRINE] } },
		{ "the mountainous area",		{ _world.regions[RegionCode::MOUNTAINOUS_AREA] } },
		{ "King Nole's cave",			{ _world.regions[RegionCode::KN_CAVE] } },
		{ "the town of Kazalt",			{ _world.regions[RegionCode::KAZALT] } },
		{ "King Nole's labyrinth",		{ _world.regions[RegionCode::KN_LABYRINTH_PRE_SPIKES], _world.regions[RegionCode::KN_LABYRINTH_POST_SPIKES], _world.regions[RegionCode::KN_LABYRINTH_RAFT_SECTOR] } },
		{ "King Nole's palace",			{ _world.regions[RegionCode::KN_PALACE] } }
	};

	// Barren / pleasant surprise hints
	for (const auto& [name, regions] : macroRegions)
	{
		bool isBarren = true;
		for (WorldRegion* region : regions)
		{
			std::vector<ItemSource*> itemSources = region->getAllItemSources();
			for (ItemSource* source : itemSources)
			{
				if (_strictlyNeededKeyItems.count(source->getItem()))
				{
					isBarren = false;
					break;
				}
			}
			if (!isBarren)
				break;
		}

		if (isBarren)
			signHintsVector.push_back("What you are looking for is not in " + name + ".");
		else
			signHintsVector.push_back("You might have a pleasant surprise wandering in " + name + ".");
	}

	// "You will / won't need {item} to finish"
	const std::vector<uint8_t> potentiallyOptionalKeyItems = { ITEM_BUYER_CARD, ITEM_EINSTEIN_WHISTLE, ITEM_ARMLET, ITEM_GARLIC, ITEM_IDOL_STONE };

	for (uint8_t keyItemID : potentiallyOptionalKeyItems)
	{
		Item* keyItem = _world.items[keyItemID];
		if (_strictlyNeededKeyItems.count(keyItem))
			signHintsVector.push_back("You will need " + keyItem->getName() + " in your quest to King Nole's treasure.");
		else
			signHintsVector.push_back(keyItem->getName() + " is useless in your quest King Nole's treasure.");
	}

	// Important items location hints
	const std::vector<uint8_t> hintableItems = {
		ITEM_SPIKE_BOOTS,		ITEM_AXE_MAGIC,		ITEM_BUYER_CARD,	ITEM_GARLIC,		ITEM_SUN_STONE,
		ITEM_EINSTEIN_WHISTLE,	ITEM_ARMLET,		ITEM_IDOL_STONE,	ITEM_KEY,			ITEM_SAFETY_PASS,
		ITEM_THUNDER_SWORD,		ITEM_HEALING_BOOTS,	ITEM_VENUS_STONE,	ITEM_STATUE_JYPTA
	};

	for (uint8_t itemID : hintableItems)
	{
		Item* hintedItem = _world.items[itemID];
		signHintsVector.push_back("You shall find " + hintedItem->getName() + " " + this->getRandomHintForItem(hintedItem) + ".");
	}
	Tools::shuffle(signHintsVector, _rng);

	uint8_t i = 0;
	for (const auto& [addr, name] : _world.hintSigns)
	{
		_world.ingameTexts[addr] = GameText(signHintsVector[i]);
		++i;
	}

	// =============== Crypt sign hints ===============
//	_world.ingameTexts[0x2797A] = GameText("This is the Crypt #0 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x27982] = GameText("This is the Crypt #1 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x2798A] = GameText("This is the Crypt #2 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x27992] = GameText("This is the Crypt #3 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x27998] = GameText("This is the Crypt #4 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279A0] = GameText("This is the Crypt #5 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279A8] = GameText("This is the Crypt #6 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279B0] = GameText("This is the Crypt #7 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279B8] = GameText("This is the Crypt #8 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279C0] = GameText("This is the Crypt #9 sign. Hints are soon to be put on this sign!");
//	_world.ingameTexts[0x279E8] = GameText("This is the Crypt #10 sign. Hints are soon to be put on this sign!");
}

std::string WorldRandomizer::getRandomHintForItem(Item* item)
{
	for (auto& [key, region] : _world.regions)
	{
		std::vector<ItemSource*> sources = region->getAllItemSources();
		for (ItemSource* source : sources)
		{
			if (source->getItem() == item)
			{
				const std::vector<std::string>& regionHints = region->getHints();
				const std::vector<std::string>& sourceHints = source->getHints();
				
				std::vector<std::string> allHints;
				allHints.insert(allHints.end(), regionHints.begin(), regionHints.end());
				allHints.insert(allHints.end(), sourceHints.begin(), sourceHints.end());
				Tools::shuffle(allHints, _rng);

				return *allHints.begin();
			}
		}
	}

	return "in an unknown place";
}

void WorldRandomizer::randomizeTiborTrees()
{
    std::vector<uint16_t> trees;
    for (const TreeMap& map : _world.treeMaps)
        trees.push_back(map.getTree());
    
	Tools::shuffle(trees, _rng);
    for (uint8_t i = 0; i < _world.treeMaps.size(); ++i)
        _world.treeMaps[i].setTree(trees[i]);
}
