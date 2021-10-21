#include "WorldRandomizer.hpp"
#include "Tools.hpp"
#include "GameText.hpp"
#include "Exceptions.hpp"
#include <algorithm>
#include <sstream>
#include <set>

WorldRandomizer::WorldRandomizer(World& world, const RandomizerOptions& options) :
	_world				(world),
	_options			(options),
	_rng				(),
	_goldItemsCount		(0)
{
	if (!options.getDebugLogPath().empty())
		_debugLog.open(options.getDebugLogPath());
	
	this->initFillerItems();
	this->initMandatoryItems();
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
	this->randomizeHints();

	if(_options.shuffleTiborTrees())
		this->randomizeTiborTrees();
	
	_debugLog.close();
}

void WorldRandomizer::initFillerItems()
{
	std::map<std::string, uint16_t> fillerItemsDescription;
	if(_options.hasCustomFillerItems())
	{
		fillerItemsDescription = _options.getFillerItems();
	}
	else
	{
		fillerItemsDescription = { 
			{"Life Stock", 80}, 	{"EkeEke", 55}, 		{"Golds", 30}, 			{"Dahl", 16}, 			
			{"Statue of Gaia", 12},	{"Detox Grass", 11}, 	{"Golden Statue", 10}, 	{"Restoration", 10}, 	
			{"Mind Repair", 7},		{"Anti Paralyze", 7}, 	{"No Item", 4},			{"Pawn Ticket", 1},
			{"Short Cake", 1},		{"Bell", 1},			{"Blue Ribbon", 1},		{"Death Statue", 1}
		};
	}

	for (auto& [itemName, quantity] : fillerItemsDescription)
	{
		if(itemName == "Golds")
		{
			_goldItemsCount += quantity;
			continue;
		}

		Item* item = _world.getItemByName(itemName);
		if(!item)
		{
			std::stringstream msg;
    		msg << "Unknown item '" << itemName << "' found in filler items.";
			throw RandomizerException(msg.str());
		}
		
		for(uint16_t i=0 ; i<quantity ; ++i)
			_fillerItems.push_back(item);
	}
}

void WorldRandomizer::initMandatoryItems()
{
	std::map<std::string, uint16_t> mandatoryItemsDescription;
	if(_options.hasCustomMandatoryItems())
	{
		mandatoryItemsDescription = _options.getMandatoryItems();
	}
	else
	{
		mandatoryItemsDescription = {
			{"Magic Sword", 1},		{"Thunder Sword", 1}, 	{"Sword of Ice", 1}, 	{"Sword of Gaia", 1},
			{"Steel Breast", 1}, 	{"Chrome Breast", 1}, 	{"Shell Breast", 1}, 	{"Hyper Breast", 1},
			{"Healing Boots", 1}, 	{"Iron Boots", 1}, 		{"Fireproof", 1},
			{"Mars Stone", 1}, 		{"Moon Stone", 1}, 		{"Saturn Stone", 1}, 	{"Venus Stone", 1},
			{"Oracle Stone", 1}, 	{"Statue of Jypta", 1}, {"Spell Book", 1},
		};
	}

	for (auto& [itemName, quantity] : mandatoryItemsDescription)
	{
		Item* item = _world.getItemByName(itemName);
		if(!item)
		{
			std::stringstream msg;
			msg << "Unknown item '" << itemName << "' found in mandatory items.";
			throw RandomizerException(msg.str());
		}
		
		for(uint16_t i=0 ; i<quantity ; ++i)
			_mandatoryItems.push_back(item);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
///		FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeGoldValues()
{
	constexpr uint16_t averageGoldPerChest = 35;
	constexpr double maxFactorOfTotalGoldValue = 0.16;

	uint16_t totalGoldValue = averageGoldPerChest * _goldItemsCount;

	for (uint8_t i = 0; i < _goldItemsCount ; ++i)
	{
		uint16_t goldValue;

		if (i < _goldItemsCount - 1)
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

		Item* goldItem = _world.addGoldItem(static_cast<uint8_t>(goldValue));
		if(goldItem)
			_fillerItems.push_back(goldItem);
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
	_regionsToExplore.insert(_world.regions[getSpawnLocationRegion(_options.getSpawnLocation())]);
	_exploredRegions.clear();		// Regions already processed by the exploration algorithm
	_itemSourcesToFill.clear();		// Reachable empty item sources which must be filled with a random item
	_playerInventory.clear();		// The current contents of player inventory at the given time in the exploration algorithm
	_pendingPaths.clear();			// Paths leading to potentially unexplored regions, locked behind a key item which is not yet owned by the player

	Tools::shuffle(_fillerItems, _rng);

	_debugLog << "\n-------------------------------------------------\n";
	_debugLog << "Step #0 (placing mandatory items)\n";
	this->placeMandatoryItems();

	uint32_t stepCount = 1;
	while (!_pendingPaths.empty() || !_regionsToExplore.empty())
	{
		_debugLog << "\n-------------------------------------------------\n";
		_debugLog << "Step #" << stepCount++ << "\n";

		// Explore not yet explored regions, listing all item sources and paths for further exploration and processing
		this->explorationPhase();

		// Try unlocking paths and item sources with newly discovered items in pre-filled item sources (useful for half-plando)
		this->unlockPhase();

		Tools::shuffle(_itemSourcesToFill, _rng);

		// Place one or several key items to unlock access to a path, opening new regions & item sources
		this->placeKeyItemsPhase();

		// Fill a fraction of already available sources with filler items
		if (!_itemSourcesToFill.empty())
		{
			size_t sourcesToFillCount = (size_t)(_itemSourcesToFill.size() * _options.getFillingRate());
			this->placeFillerItemsPhase(sourcesToFillCount);
		}

		// Try unlocking paths and item sources with the newly acquired key item
		this->unlockPhase();
	}

	// Place the remaining filler items, and put Ekeeke in the last empty sources
	this->placeFillerItemsPhase(_itemSourcesToFill.size(), _world.items[ITEM_EKEEKE]);

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
	_debugLog << "\t - " << _pendingPaths.size() << " pending paths\n";
}

void WorldRandomizer::placeMandatoryItems()
{
	// Mandatory items are filler items which are always placed first in the randomization, no matter what
	Tools::shuffle(_mandatoryItems, _rng);

	std::vector<ItemSource*> allEmptyItemSources;
	for (auto& [key, source] : _world.itemSources)
		if(!source->getItem())
			allEmptyItemSources.push_back(source);
	Tools::shuffle(allEmptyItemSources, _rng);

	for (Item* itemToPlace : _mandatoryItems)
	{
		for (ItemSource* source : allEmptyItemSources)
		{
			if (!source->getItem() && source->isItemCompatible(itemToPlace))
			{
				source->setItem(itemToPlace);
				_debugLog << "\t > Placing mandatory item [" << itemToPlace->getName() << "] in \"" << source->getName() << "\"\n";
				break;
			}
		}
	}
}

void WorldRandomizer::placeFillerItemsPhase(size_t count, Item* lastResortFiller)
{
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
				_fillerItems.erase(_fillerItems.begin() + j);
				_itemSourcesToFill.erase(_itemSourcesToFill.begin());
				break;
			}
		}

		if(itemSource->getItem() == nullptr)
		{
			// No valid item could be put inside the itemSource...
			if(lastResortFiller)
			{
				// Fill with the "last resort filler" if provided
				itemSource->setItem(lastResortFiller);
				_itemSourcesToFill.erase(_itemSourcesToFill.begin());
			}
			else
			{
				// No last resort provided, put this item source on the back of the list
				_itemSourcesToFill.erase(_itemSourcesToFill.begin());
				_itemSourcesToFill.push_back(itemSource);
			}
		}

		if(itemSource->getItem())
		{
			_debugLog << "\t\t - Filling \"" << itemSource->getName() << "\" with [" << itemSource->getItem()->getName() << "]\n";
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
			if(itemSource->getItem())
				_playerInventory.push_back(itemSource->getItem());	// Non-empty item sources populate player inventory (useful for plandos)
			else
				_itemSourcesToFill.push_back(itemSource);
		}

		// List outgoing paths
		for (WorldPath* outgoingPath : exploredRegion->getOutgoingPaths())
		{
			// If destination is already pending exploration (through another path) or has already been explored, just ignore it
			WorldRegion* destination = outgoingPath->getDestination();
			if (_regionsToExplore.contains(destination) || _exploredRegions.contains(destination))
				continue;

			if (outgoingPath->getMissingItemsToCross(_playerInventory).empty())
			{
				// For crossable paths, add destination to the list of regions to explore
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

void WorldRandomizer::placeKeyItemsPhase()
{
	if (_pendingPaths.empty())
		return;

	// List all unowned key items, and pick a random one among them
	std::vector<WorldPath*> blockedPaths;
	for (WorldPath* pendingPath : _pendingPaths)
	{
		if(!pendingPath->getMissingItemsToCross(_playerInventory).empty())
		{
			// If items are missing to cross this path, add as many entries as the weight of the path to the blockedPaths array
			for(int i=0 ; i<pendingPath->getRandomWeight() ; ++i)
				blockedPaths.push_back(pendingPath);
		}
	}

	Tools::shuffle(blockedPaths, _rng);
	WorldPath* pathToOpen = blockedPaths[0];
	std::vector<Item*> missingKeyItems = pathToOpen->getMissingItemsToCross(_playerInventory);
	for(Item* keyItemToPlace : missingKeyItems)
	{
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
		_playerInventory.push_back(keyItemToPlace);
	}
}

void WorldRandomizer::unlockPhase()
{
	// Look for unlockable paths...
	for (size_t i=0 ; i < _pendingPaths.size() ; ++i)
	{
		WorldPath* pendingPath = _pendingPaths[i];

		if (pendingPath->getMissingItemsToCross(_playerInventory).empty())
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

void WorldRandomizer::randomizeHints()
{
	// Lithograph hints
	if(_options.getJewelCount() > MAX_INDIVIDUAL_JEWELS)
	{
		std::vector<ItemSource*> allSourcesContainingJewels = _world.getItemSourcesContainingItem(_world.items[ITEM_RED_JEWEL]);
		for(ItemSource* source : allSourcesContainingJewels)
			_world.jewelHints.push_back("A jewel is " + this->getRandomHintForItemSource(source) + ".");
	}
	else
	{
		if(_options.getJewelCount() >= 1)
			_world.jewelHints.push_back("Red Jewel is " + this->getRandomHintForItem(_world.items[ITEM_RED_JEWEL]) + ".");
		if(_options.getJewelCount() >= 2)
			_world.jewelHints.push_back("Purple Jewel is " + this->getRandomHintForItem(_world.items[ITEM_PURPLE_JEWEL]) + ".");
		if(_options.getJewelCount() >= 3)
			_world.jewelHints.push_back("Green Jewel is " + this->getRandomHintForItem(_world.items[ITEM_GREEN_JEWEL]) + ".");
		if(_options.getJewelCount() >= 4)
			_world.jewelHints.push_back("Blue Jewel is " + this->getRandomHintForItem(_world.items[ITEM_BLUE_JEWEL]) + ".");
		if(_options.getJewelCount() >= 5)
			_world.jewelHints.push_back("Yellow Jewel is " + this->getRandomHintForItem(_world.items[ITEM_YELLOW_JEWEL]) + ".");
	}

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
		forbiddenFortuneTellerItem, _world.items[ITEM_RED_JEWEL], _world.items[ITEM_PURPLE_JEWEL],
		_world.items[ITEM_GREEN_JEWEL], _world.items[ITEM_BLUE_JEWEL], _world.items[ITEM_YELLOW_JEWEL]
	};

	// Also excluding items strictly needed to get to Oracle Stone's location
	WorldRegion* itemRegion = _world.getRegionForItem(_world.items[ITEM_ORACLE_STONE]);
	if(itemRegion)
	{
		UnsortedSet<Item*> strictlyNeededKeyItemsForOracleStone = this->analyzeStrictlyRequiredKeyItemsForRegion(itemRegion);
		for (Item* item : strictlyNeededKeyItemsForOracleStone)
			forbiddenOracleStoneItems.insert(item);
	}

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
	
	_world.oracleStoneHint = "The stone looks blurry. It looks like it won't be of any use...";
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
		ITEM_SUN_STONE,			ITEM_KEY,			ITEM_SAFETY_PASS,	ITEM_LOGS,
		ITEM_GOLA_EYE,			ITEM_GOLA_NAIL,		ITEM_GOLA_FANG,		ITEM_GOLA_HORN
	};
	
	auto it = std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedFortuneItem->getID());
	if(it != hintableItemLocations.end())
		hintableItemLocations.erase(it);

	it = std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedOracleStoneItem->getID());
	if(it != hintableItemLocations.end())
		hintableItemLocations.erase(it);

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
		// "You will / won't need {item} to finish" (25%)
		else if (randomNumber < 0.55 && nextElligibleHintableItemsNecessityPos >=0)
		{
			Item* hintedItem = _world.items[hintableItemsNecessity.at(nextElligibleHintableItemsNecessityPos)];
			if (_strictlyNeededKeyItems.contains(hintedItem))
				hintText = "You will need " + hintedItem->getName() + " in your quest to King Nole's treasure.";
			else
				hintText = hintedItem->getName() + " is useless in your quest King Nole's treasure.";

			hintableItemsNecessity.erase(hintableItemsNecessity.begin() + nextElligibleHintableItemsNecessityPos);
		}
		// "You shall find {item} in {place}" (45%)
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

		sign->setText(hintText);
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
	std::vector<ItemSource*> sources = _world.getItemSourcesContainingItem(item);
	if(sources.empty())
		return "in an unknown place";

	Tools::shuffle(sources, _rng);
	ItemSource* randomSource = *sources.begin();
	return this->getRandomHintForItemSource(randomSource);
}

std::string WorldRandomizer::getRandomHintForItemSource(ItemSource* itemSource)
{
	const std::vector<std::string>& regionHints = itemSource->getRegion()->getHints();
	const std::vector<std::string>& sourceHints = itemSource->getHints();
		
	std::vector<std::string> allHints;
	allHints.insert(allHints.end(), regionHints.begin(), regionHints.end());
	allHints.insert(allHints.end(), sourceHints.begin(), sourceHints.end());
	
	if(allHints.empty())
		return "in an unknown place";

	Tools::shuffle(allHints, _rng);
	return *allHints.begin();
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
