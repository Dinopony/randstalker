#include "WorldRandomizer.h"
#include "Tools.h"
#include "GameText.h"
#include <algorithm>
#include <sstream>

constexpr auto FILLING_RATE = 0.20;


WorldRandomizer::WorldRandomizer(World& world, const RandomizerOptions& options) :
	_world				(world),
	_options			(options),
	_rng				(options.getSeed()),
	_logFile			()
{
	_logFile.open(options.getSpoilerLogPath());
	_options.logToFile(_logFile);
}

void WorldRandomizer::randomize()
{
	this->randomizeGoldValues();
//	this->randomizeDarkRooms();

	this->randomizeItems();
	
	this->setSpawnPoint();
	this->randomizeLithographHints();

	if(_options.shuffleTiborTrees())
		this->randomizeTiborTrees();
}

void WorldRandomizer::setSpawnPoint()
{
	SpawnLocation spawnLocation = _options.getSpawnLocation();

	if (spawnLocation == SpawnLocation::RANDOM)
	{
		std::vector<SpawnLocation> possibleSpawnRegions = { SpawnLocation::MASSAN, SpawnLocation::GUMI, SpawnLocation::RYUMA };
		Tools::shuffle(possibleSpawnRegions, _rng);
		spawnLocation = possibleSpawnRegions[0];
	}

	if (spawnLocation == SpawnLocation::MASSAN)
	{
		_world.spawnMapID = 0x258;
		_world.spawnX = 0x1F;
		_world.spawnZ = 0x19;
	}
	else if (spawnLocation == SpawnLocation::GUMI)
	{
		_world.spawnMapID = 0x25E;
		_world.spawnX = 0x10;
		_world.spawnZ = 0x0F;
	}
	else if (spawnLocation == SpawnLocation::RYUMA)
	{
		_world.spawnMapID = 0x268;
		_world.spawnX = 0x11;
		_world.spawnZ = 0x14;
	}
}

void WorldRandomizer::randomizeGoldValues()
{
	constexpr uint16_t averageGoldPerChest = 35;
	constexpr double maxFactorOfTotalGoldValue = 0.16;

	uint16_t totalGoldValue = averageGoldPerChest * GOLD_SOURCES_COUNT;

	for (uint8_t i = 0; i < GOLD_SOURCES_COUNT ; ++i)
	{
		Item* goldItem = _world.items[ITEM_GOLDS_START + i];
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

		goldItem->setName(goldName.str());
		goldItem->setGoldWorth((uint8_t)goldValue);
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
	_world.darkenedRegion = *darkenableRegions.begin();

	_logFile << "Dark region: " << _world.darkenedRegion->getName() << "\n\n";

	const std::vector<WorldPath*>& ingoingPaths = _world.darkenedRegion->getIngoingPaths();
	for (WorldPath* path : ingoingPaths)
		path->addRequiredItem(_world.items[ITEM_LANTERN]);
}

void WorldRandomizer::initPriorityItems()
{
	// Priority items are filler items which are always placed first in the randomization, no matter what
	_priorityItems.push_back(_world.items[ITEM_MAGIC_SWORD]);
	_priorityItems.push_back(_world.items[ITEM_THUNDER_SWORD]);
	_priorityItems.push_back(_world.items[ITEM_ICE_SWORD]);
	_priorityItems.push_back(_world.items[ITEM_GAIA_SWORD]);

	_priorityItems.push_back(_world.items[ITEM_STEEL_BREAST]);
	_priorityItems.push_back(_world.items[ITEM_CHROME_BREAST]);
	_priorityItems.push_back(_world.items[ITEM_SHELL_BREAST]);
	_priorityItems.push_back(_world.items[ITEM_HYPER_BREAST]);

	_priorityItems.push_back(_world.items[ITEM_HEALING_BOOTS]);
	_priorityItems.push_back(_world.items[ITEM_IRON_BOOTS]);
	_priorityItems.push_back(_world.items[ITEM_FIREPROOF_BOOTS]);

	_priorityItems.push_back(_world.items[ITEM_MARS_STONE]);
	_priorityItems.push_back(_world.items[ITEM_MOON_STONE]);
	_priorityItems.push_back(_world.items[ITEM_SATURN_STONE]);
	_priorityItems.push_back(_world.items[ITEM_VENUS_STONE]);

	_priorityItems.push_back(_world.items[ITEM_DEATH_STATUE]);
	_priorityItems.push_back(_world.items[ITEM_BELL]);
	_priorityItems.push_back(_world.items[ITEM_LANTERN]);
	_priorityItems.push_back(_world.items[ITEM_STATUE_JYPTA]);
}

void WorldRandomizer::initFillerItems()
{
	_fillerItems.push_back(_world.items[ITEM_GARLIC]);
	_fillerItems.push_back(_world.items[ITEM_PAWN_TICKET]);
	_fillerItems.push_back(_world.items[ITEM_SHORT_CAKE]);
	_fillerItems.push_back(_world.items[ITEM_SPELL_BOOK]);
	_fillerItems.push_back(_world.items[ITEM_BLUE_RIBBON]);
	_fillerItems.push_back(_world.items[ITEM_ORACLE_STONE]);

	for (uint8_t i = 0; i < 75; ++i)
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
		_fillerItems.push_back(_world.items[ITEM_GOLDS_START+i]);

	for (uint8_t i = 0; i < 4; ++i)
		_fillerItems.push_back(_world.items[ITEM_NONE]);
}

void WorldRandomizer::randomizeItems()
{
	this->initPriorityItems();
	Tools::shuffle(_priorityItems, _rng);

	this->initFillerItems();
	Tools::shuffle(_fillerItems, _rng);

	std::vector<Item*> playerInventory;

	this->fillSourcesWithPriorityItems();

	for (int stepCount = 1;  ; ++stepCount)
	{
		_logFile << "Step #" << stepCount << "\n";

		// Evaluate accessible regions & stuff
		std::vector<Item*> keyItemsNeededToProgress;
		std::vector<AbstractItemSource*> reachableItemSources;
		std::vector<WorldRegion*> reachableRegions = this->evaluateReachableRegions(playerInventory, keyItemsNeededToProgress, reachableItemSources);
		if (keyItemsNeededToProgress.empty())
			break;

		Tools::shuffle(reachableItemSources, _rng);

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
		int additionnalSourcesToFill = static_cast<int>(reachableItemSources.size() * FILLING_RATE);
		_logFile << "\t > Filling " << additionnalSourcesToFill << " additionnal sources with filler items\n";
		this->fillSourcesWithFillerItems(reachableItemSources.begin(), reachableItemSources.begin() + additionnalSourcesToFill);
	}

	// If no key items are remaining, this means we are in go-mode and we can fill all remaining item sources
	std::vector<AbstractItemSource*> emptyItemSources;
	for(auto& [key, source] : _world.itemSources)
		if(!source->getItem())
			emptyItemSources.push_back(source);

	_logFile << "\t > Trying to fill the " << emptyItemSources.size() << " remaining sources...\n";
	this->fillSourcesWithFillerItems(emptyItemSources.begin(), emptyItemSources.end());

	// Write down the complete item list in the log file
	this->writeItemSourcesBreakdownInLog();
}

std::vector<WorldRegion*> WorldRandomizer::evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources)
{
	std::vector<WorldRegion*> returnedRegions;
	std::vector<WorldRegion*> regionsToProcess;
	regionsToProcess.push_back(_world.regions[RegionCode::MASSAN]);

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
		
		const std::vector<std::pair<AbstractItemSource*, Item*>>& restrictedItemSourcesInRegion = processedRegion->getRestrictedItemSources();
		for(auto& [source, requiredItem] : restrictedItemSourcesInRegion)
		{
			if (source->getItem())
				continue;

			// If player has the required item, add the source to reachable sources
			if(std::find(playerInventory.begin(), playerInventory.end(), requiredItem) != playerInventory.end())
			{
				out_reachableSources.push_back(source);
			}
			// Otherwise, add the required item to the list of needed items to progress (if it's not already inside)
			else if (std::find(out_keyItems.begin(), out_keyItems.end(), requiredItem) == out_keyItems.end())
			{
				out_keyItems.push_back(requiredItem);
			}
		}

		// Analyze outgoing paths to check for other regions to explore
		const std::vector<WorldPath*>& outgoingPaths = processedRegion->getOutgoingPaths();
		for (WorldPath* path : outgoingPaths)
		{
			WorldRegion* destination = path->getDestination();

			std::vector<Item*> requiredKeyItems = path->getRequiredItems();
			bool canReachRegion = true;
			if (!requiredKeyItems.empty())
			{
				for (Item* requiredItem : requiredKeyItems)
				{
					bool hasItem = false;
					for (Item* ownedItem : playerInventory)
					{
						if (ownedItem == requiredItem)
						{
							hasItem = true;
							break;
						}
					}

					if (!hasItem)
					{
						if (std::find(out_keyItems.begin(), out_keyItems.end(), requiredItem) == out_keyItems.end())
							out_keyItems.push_back(requiredItem);
						canReachRegion = false;
						break;
					}
				}
			}

			if (canReachRegion)
			{
				bool regionAlreadyProcessed = std::find(returnedRegions.begin(), returnedRegions.end(), destination) != returnedRegions.end();
				bool regionAlreadyQueuedForProcessing = std::find(regionsToProcess.begin(), regionsToProcess.end(), destination) != regionsToProcess.end();
				if( !regionAlreadyProcessed && !regionAlreadyQueuedForProcessing )
					regionsToProcess.push_back(destination);
			}
		}
	}

	return returnedRegions;
}

void WorldRandomizer::fillSourcesWithPriorityItems()
{
	_logFile << "Step #0 (placing priority items)\n";

	std::vector<AbstractItemSource*> allItemSources;
	for (auto& [key, source] : _world.itemSources)
		allItemSources.push_back(source);
	Tools::shuffle(allItemSources, _rng);

	while (!_priorityItems.empty())
	{
		Item* itemToPlace = *_priorityItems.begin();

		for (AbstractItemSource* source : allItemSources)
		{
			if (!source->getItem() && source->isItemCompatible(itemToPlace))
			{
				source->setItem(itemToPlace);
				_logFile << "\t > Placing priority item [" << itemToPlace->getName() << "] in \"" << source->getName() << "\"\n";
				break;
			}
		}

		_priorityItems.erase(_priorityItems.begin());
	}
}

void WorldRandomizer::fillSourcesWithFillerItems(std::vector<AbstractItemSource*>::iterator begin, std::vector<AbstractItemSource*>::iterator end)
{
	for (auto it = begin ; it != end && !_fillerItems.empty() ; ++it)
	{
		AbstractItemSource* itemSource = *it;
		Item* randomFillerItem = nullptr;
		for (uint32_t j = 0; j < _fillerItems.size(); ++j)
		{
			if (itemSource->isItemCompatible(_fillerItems[j]))
			{
				randomFillerItem = _fillerItems[j];
				_fillerItems.erase(_fillerItems.begin() + j);
				break;
			}
		}

		if (!randomFillerItem)
			randomFillerItem = _world.items[ITEM_EKEEKE];

		itemSource->setItem(randomFillerItem);
		_logFile << "\t\t >>> Filling \"" << itemSource->getName() << "\" with [" << randomFillerItem->getName() << "]\n";
	}
}

void WorldRandomizer::writeItemSourcesBreakdownInLog()
{
	for (auto& [key, region] : _world.regions)
	{
		std::vector<AbstractItemSource*> sources = region->getAllItemSources();
		if (sources.empty())
			continue;

		_logFile << "\n-------------------------------\n";
		_logFile << "\t" << region->getName() << "\n\n";

		for (AbstractItemSource* source : sources)
		{
			_logFile << "[" << (source->getItem() ? source->getItem()->getName() : "No Item (out of items)") << "] in \"" << source->getName() << "\"\n";
		}
	}

	_logFile << "\n-------------------------------\n";
	_logFile << "Unplaced items:" << "\n";

	for (Item* item : _fillerItems)
		_logFile << "- [" << item->getName() << "]\n";
}

void WorldRandomizer::randomizeLithographHints()
{
	std::string redJewelHint = this->getRandomHintForItem(_world.items[ITEM_RED_JEWEL]);
	std::string purpleJewelHint = this->getRandomHintForItem(_world.items[ITEM_PURPLE_JEWEL]);

	std::string completeHint = "Red Jewel is " + redJewelHint + ".\t\nPurple Jewel is " + purpleJewelHint + ".\t";
	_world.lithographHint = GameText(completeHint);
}

std::string WorldRandomizer::getRandomHintForItem(Item* item)
{
	for (auto& [key, region] : _world.regions)
	{
		std::vector<AbstractItemSource*> sources = region->getAllItemSources();
		for (AbstractItemSource* source : sources)
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

    _logFile << "\n-------------------------------\n";
    _logFile << "Tibor trees:" << "\n";

    std::pair<TreeMap*, TreeMap*> pairs[5];
    for (TreeMap& map : _world.treeMaps)
    {
        uint16_t treeMapID = map.getTree();
        if (treeMapID == 0x0200)		pairs[0].first = &map;
        else if (treeMapID == 0x0216)	pairs[0].second = &map;
        else if (treeMapID == 0x021B)	pairs[1].first = &map;
        else if (treeMapID == 0x0219)	pairs[1].second = &map;
        else if (treeMapID == 0x0218)	pairs[2].first = &map;
        else if (treeMapID == 0x0201)	pairs[2].second = &map;
        else if (treeMapID == 0x021A)	pairs[3].first = &map;
        else if (treeMapID == 0x0217)	pairs[3].second = &map;
        else if (treeMapID == 0x01FF)	pairs[4].first = &map;
        else if (treeMapID == 0x01FE)	pairs[4].second = &map;
    }

    for (int i = 0; i < 5; ++i)
        _logFile << "- " << pairs[i].first->getName() << " <===> " << pairs[i].second->getName() << "\n";
}
