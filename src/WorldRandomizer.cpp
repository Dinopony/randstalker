#include "WorldRandomizer.hpp"
#include "Tools.hpp"
#include "GameText.hpp"
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
	this->randomizeSpawnLocation();
	this->randomizeGoldValues();
	this->randomizeDarkRooms();

	// 2nd pass: randomizing items
	_rng.seed(rngSeed);
	this->randomizeItems();
	this->analyzeStrictlyRequiredKeyItems();
	
	// 3rd pass: randomizations happening AFTER randomizing items
	_rng.seed(rngSeed);
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
			double proportion = (double) _rng() / (double) _rng.max();
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
	constexpr double FILLING_RATE = 0.20;

	_regionsToExplore.insert(_world.regions[RegionCode::MASSAN]);
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
		const std::vector<ItemSource*> itemSources = exploredRegion->getItemSources();
		for (ItemSource* itemSource : itemSources)
		{
			Item* requiredItem = itemSource->getRequiredItem();
			if (requiredItem)
				_pendingItemSources[itemSource] = requiredItem;
			else if(itemSource->getItem())
				_playerInventory.insert(itemSource->getItem());	// Non-empty item sources populate player inventory (useful for plandos)
			else
				_itemSourcesToFill.push_back(itemSource);
		}

		// List outgoing paths
		for (WorldPath* outgoingPath : exploredRegion->getOutgoingPaths())
		{
			if (outgoingPath->canBeCrossedWithInventory(_playerInventory))
			{
				// For crossable paths, add destinations in regions to explore if not already explored / to process
				WorldRegion* destination = outgoingPath->getDestination();
				if (!_regionsToExplore.contains(destination) && !_exploredRegions.contains(destination))
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
			if (!_playerInventory.contains(requiredItem))
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
			if (!_regionsToExplore.contains(destination) && !_exploredRegions.contains(destination))
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

		if (_playerInventory.contains(requiredItem))
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
	_strictlyNeededKeyItems = this->analyzeStrictlyRequiredKeyItemsForRegion(_world.regions[RegionCode::ENDGAME]);

	// Output required item list to debug log if we are in debug mode
	if (_debugLog)
	{
		_debugLog << "\n-------------------------------\n";
		_debugLog << "\tItems required to finish the seed" << "\n\n";
		for (Item* item : _strictlyNeededKeyItems)
			_debugLog << "\t- " << item->getName() << "\n";
	}
}

UnsortedSet<Item*> WorldRandomizer::analyzeStrictlyRequiredKeyItemsForItem(Item* item)
{
	WorldRegion* itemRegion = _world.getRegionForItem(item);
	UnsortedSet<Item*> strictlyRequiredKeyItemsForItem = this->analyzeStrictlyRequiredKeyItemsForRegion(itemRegion);
	
	ItemSource* itemSource = _world.getItemSourceForItem(item);
	if(itemSource && itemSource->getRequiredItem())
		strictlyRequiredKeyItemsForItem.insert(itemSource->getRequiredItem());

	return strictlyRequiredKeyItemsForItem;
}

UnsortedSet<Item*> WorldRandomizer::analyzeStrictlyRequiredKeyItemsForRegion(WorldRegion* region)
{
	const UnsortedSet<Item*> optionalItems = { _world.items[ITEM_LITHOGRAPH], _world.items[ITEM_LANTERN] };

	// We perform a backwards analysis here starting from endgame region and determining which key items are strictly needed to finish the seed
	UnsortedSet<Item*> strictlyNeededKeyItems;
	UnsortedSet<WorldRegion*> regionsToExplore = { region };
	UnsortedSet<WorldRegion*> alreadyExploredRegions;
	UnsortedSet<Item*> itemsToLocate;

	while (!regionsToExplore.empty())
	{
		WorldRegion* exploredRegion = *regionsToExplore.begin();
		regionsToExplore.erase(exploredRegion);
		alreadyExploredRegions.insert(exploredRegion);

		const std::vector<WorldPath*>& pathsToRegion = exploredRegion->getIngoingPaths();
		for (WorldPath* path : pathsToRegion)
		{
			for (Item* neededItem : path->getRequiredItems())
				if (!optionalItems.contains(neededItem))
					itemsToLocate.insert(neededItem);

			WorldRegion* originRegion = path->getOrigin();
			if (!alreadyExploredRegions.contains(originRegion))
				regionsToExplore.insert(originRegion);
		}

		while (regionsToExplore.empty() && !itemsToLocate.empty())
		{
			Item* keyItemToLocate = *itemsToLocate.begin();
			itemsToLocate.erase(keyItemToLocate);
			if (!strictlyNeededKeyItems.contains(keyItemToLocate))
			{
				strictlyNeededKeyItems.insert(keyItemToLocate);

				for (const auto& [code, source] : _world.itemSources)
				{
					if (source->getItem() == keyItemToLocate)
					{
						WorldRegion* region = source->getRegion();
						if (!alreadyExploredRegions.contains(region))
							regionsToExplore.insert(region);

						if (source->getRequiredItem())
							itemsToLocate.insert(source->getRequiredItem());

						break;
					}
				}
			}
		}
	}

	return strictlyNeededKeyItems;
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
	// Lithograph hints
	_world.redJewelHint = "Red Jewel is " + this->getRandomHintForItem(_world.items[ITEM_RED_JEWEL]) + ".";
	_world.purpleJewelHint = "Purple Jewel is " + this->getRandomHintForItem(_world.items[ITEM_PURPLE_JEWEL]) + ".";

	// King Nole Cave "where is lithograph" hint sign
	_world.whereIsLithographHint = "The lithograph will help you finding the jewels. It is " 
		+ this->getRandomHintForItem(_world.items[ITEM_LITHOGRAPH]) + ".";

	// Fortune Teller hint
	Item* hintedFortuneItem = this->randomizeFortuneTellerHint();
	Item* hintedOracleStoneItem = this->randomizeOracleStoneHint(hintedFortuneItem);

	// Sign hints
	this->randomizeSignHints(hintedFortuneItem, hintedOracleStoneItem);
}

Item* WorldRandomizer::randomizeFortuneTellerHint()
{
	std::vector<uint8_t> hintableItemsByFortuneTeller = { ITEM_GOLA_EYE, ITEM_GOLA_NAIL, ITEM_GOLA_FANG, ITEM_GOLA_HORN };
	Tools::shuffle(hintableItemsByFortuneTeller, _rng);
	
	Item* fortuneHintedItem = _world.items[*(hintableItemsByFortuneTeller.begin())];

	std::string fortuneItemName;
	if (fortuneHintedItem == _world.items[ITEM_GOLA_EYE])
		fortuneItemName = "an eye";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_NAIL])
		fortuneItemName = "a nail";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_FANG])
		fortuneItemName = "a fang";
	else if (fortuneHintedItem == _world.items[ITEM_GOLA_HORN])
		fortuneItemName = "a horn";

	_world.fortuneTellerHint = "I see " + fortuneItemName + " " + this->getRandomHintForItem(fortuneHintedItem) + ".";

	return fortuneHintedItem;
}

Item* WorldRandomizer::randomizeOracleStoneHint(Item* forbiddenFortuneTellerItem)
{
	UnsortedSet<Item*> forbiddenOracleStoneItems = {
		forbiddenFortuneTellerItem, _world.items[ITEM_RED_JEWEL], _world.items[ITEM_PURPLE_JEWEL]
	};

	// Also excluding items strictly needed to get to Oracle Stone's location
	UnsortedSet<Item*> strictlyNeededKeyItemsForOracleStone = this->analyzeStrictlyRequiredKeyItemsForItem(_world.items[ITEM_ORACLE_STONE]);
	for (Item* item : strictlyNeededKeyItemsForOracleStone)
		forbiddenOracleStoneItems.insert(item);

	std::vector<Item*> hintableItems;
	for (Item* item : _strictlyNeededKeyItems)
	{
		if(!forbiddenOracleStoneItems.contains(item))
			hintableItems.push_back(item);
	}
	
	if (!hintableItems.empty())
	{
		Tools::shuffle(hintableItems, _rng);
		Item* itemInOracleStoneHint = hintableItems[0];
		_world.oracleStoneHint = "You will need " + itemInOracleStoneHint->getName() + ". It is " + this->getRandomHintForItem(itemInOracleStoneHint) + ".";
		return itemInOracleStoneHint;
	}
	
	_world.oracleStoneHint = "The stone looks blurry. It looks like it won't be of any use this time...";
	return nullptr;
}


void WorldRandomizer::randomizeSignHints(Item* hintedFortuneItem, Item* hintedOracleStoneItem)
{
	// A shuffled list of macro regions, useful for the "barren / useful region" hints
	std::vector<WorldMacroRegion*> macroRegionsAvailableForHints;
	for (WorldMacroRegion* macroRegion : _world.macroRegions)
		macroRegionsAvailableForHints.push_back(macroRegion);
	Tools::shuffle(macroRegionsAvailableForHints, _rng);

	// A shuffled list of potentially optional items, useful for the "this item will be useful / useless" hints
	std::vector<uint8_t> hintableItemsNecessity = {
		ITEM_BUYER_CARD, ITEM_EINSTEIN_WHISTLE, ITEM_ARMLET, ITEM_GARLIC, ITEM_IDOL_STONE, ITEM_CASINO_TICKET, ITEM_LOGS
	};
	Tools::shuffle(hintableItemsNecessity, _rng);

	// A shuffled list of items which location is interesting, useful for the "item X is in Y" hints
	std::vector<uint8_t> hintableItemLocations = {
		ITEM_SPIKE_BOOTS,		ITEM_AXE_MAGIC,		ITEM_BUYER_CARD,	ITEM_GARLIC,
		ITEM_EINSTEIN_WHISTLE,	ITEM_ARMLET,		ITEM_IDOL_STONE,
		ITEM_THUNDER_SWORD,		ITEM_HEALING_BOOTS,	ITEM_VENUS_STONE,	ITEM_STATUE_JYPTA,
		ITEM_SUN_STONE,			ITEM_KEY,			ITEM_SAFETY_PASS,
		ITEM_GOLA_EYE,			ITEM_GOLA_NAIL,		ITEM_GOLA_FANG,		ITEM_GOLA_HORN
	};
	hintableItemLocations.erase(std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedFortuneItem->getID()));
	hintableItemLocations.erase(std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedOracleStoneItem->getID()));
	Tools::shuffle(hintableItemLocations, _rng);

	for (HintSign* sign : _world.hintSigns)
	{
		std::string hintText;
		double randomNumber = (double) _rng() / (double) _rng.max();
		WorldRegion* signRegion = sign->getRegion();
		UnsortedSet<Item*> itemsAlreadyObtainedAtSign = this->analyzeStrictlyRequiredKeyItemsForRegion(signRegion);
		int nextElligibleHintableItemsNecessityPos = this->getNextElligibleHintableItemPos(hintableItemsNecessity, itemsAlreadyObtainedAtSign);

		// "Barren / pleasant surprise" (30%)
		if (randomNumber < 0.3 && !macroRegionsAvailableForHints.empty())
		{
			WorldMacroRegion* macroRegion = *macroRegionsAvailableForHints.begin();
			if (macroRegion->isBarren(_strictlyNeededKeyItems))
				hintText = "What you are looking for is not in " + macroRegion->getName() + ".";
			else
				hintText = "You might have a pleasant surprise wandering in " + macroRegion->getName() + ".";

			macroRegionsAvailableForHints.erase(macroRegionsAvailableForHints.begin());
		}
		// "You will / won't need {item} to finish" (10%)
		else if (randomNumber < 0.5 && nextElligibleHintableItemsNecessityPos >=0)
		{
			Item* hintedItem = _world.items[hintableItemsNecessity.at(nextElligibleHintableItemsNecessityPos)];
			if (_strictlyNeededKeyItems.contains(hintedItem))
				hintText = "You will need " + hintedItem->getName() + " in your quest to King Nole's treasure.";
			else
				hintText = hintedItem->getName() + " is useless in your quest King Nole's treasure.";

			hintableItemsNecessity.erase(hintableItemsNecessity.begin() + nextElligibleHintableItemsNecessityPos);
		}
		// "You shall find {item} in {place}" (60%)
		else if (!hintableItemLocations.empty())
		{

			Item* hintedItem = nullptr;
			for (uint32_t i = 0; i < hintableItemLocations.size(); ++i)
			{
				Item* testedItem = _world.items[hintableItemLocations[i]];
				if (!itemsAlreadyObtainedAtSign.contains(testedItem))
				{
					hintedItem = testedItem;
					hintableItemLocations.erase(hintableItemLocations.begin() + i);
					break;
				}
			}

			if (hintedItem) {
				hintText = "You shall find " + hintedItem->getName() + " " + this->getRandomHintForItem(hintedItem) + ".";
			} else {
				hintText = "This sign has been damaged in a way that makes it unreadable.";
			}
		}

		_world.textLines[sign->getTextID()] = GameText(hintText).getOutput();
	}
}

uint32_t WorldRandomizer::getNextElligibleHintableItemPos(std::vector<uint8_t> hintableItemsNecessity, UnsortedSet<Item*> itemsAlreadyObtainedAtSign)
{
	for (uint32_t i = 0; i < hintableItemsNecessity.size(); ++i)
	{
		Item* testedItem = _world.items[hintableItemsNecessity[i]];
		if (!itemsAlreadyObtainedAtSign.contains(testedItem))
		{
			return i;
		}
	}
	return -1;
}

std::string WorldRandomizer::getRandomHintForItem(Item* item)
{
	for (auto& [key, region] : _world.regions)
	{
		std::vector<ItemSource*> sources = region->getItemSources();
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
