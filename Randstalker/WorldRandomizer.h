#pragma once

#include <vector>
#include <string>
#include <random>

#include "ItemCodes.h"
#include "ItemSourceCodes.h"
#include "RegionCodes.h"
#include "AbstractItemSource.h"
#include "RandomizerOptions.h"
#include "Item.h"
#include "World.h"
#include "WorldRegion.h"

class NoAppropriateItemSourceException : public std::exception {};

class WorldRandomizer
{
public:
	WorldRandomizer(World& world, const RandomizerOptions& options);
	~WorldRandomizer();

	void randomize();
	void writeSpoilerLog();

private:
	// First pass randomizations (before items)
	void randomizeGoldValues();
	void randomizeDarkRooms();

	// Second pass randomizations (items)
	void randomizeItems();

	void placePriorityItems();
	void initFillerItems();

	void placeFillerItemsPhase(size_t count = 0);
	void explorationPhase();
	void placeKeyItemPhase();
	void unlockPhase();


	// Third pass randomizations (after items)
	void randomizeSpawnLocation();
	void randomizeHints();
	std::string getRandomHintForItem(Item* item);
	void randomizeTiborTrees();

private:
	World& _world;
	const RandomizerOptions& _options;

	std::ofstream _debugLog;
	std::mt19937 _rng;

	std::set<WorldRegion*> _regionsToExplore;
	std::set<WorldRegion*> _exploredRegions;
	std::vector<AbstractItemSource*> _itemSourcesToFill;
	std::set<Item*> _playerInventory;
	std::map<AbstractItemSource*, Item*> _pendingItemSources;
	std::vector<WorldPath*> _pendingPaths;
	std::vector<Item*> _fillerItems;

	std::vector<Item*> _keyItems;
};
