#pragma once

#include <vector>
#include <string>
#include <random>

#include "ItemCodes.h"
#include "ItemSourceCodes.h"
#include "RegionCodes.h"
#include "ItemSources.h"
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

private:
	// First pass randomizations (before items)
	void randomizeGoldValues();
	void randomizeDarkRooms();

	// Second pass randomizations (items)
	void randomizeItems();
	void analyzeStrictlyRequiredKeyItems();
	std::set<Item*> analyzeStrictlyRequiredKeyItemsForRegion(WorldRegion* region);

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
	WorldRegion* getRegionForItem(Item* item);
	void randomizeTiborTrees();

private:
	World& _world;
	const RandomizerOptions& _options;

	std::ofstream _debugLog;
	std::mt19937 _rng;

	std::set<WorldRegion*> _regionsToExplore;
	std::set<WorldRegion*> _exploredRegions;
	std::vector<ItemSource*> _itemSourcesToFill;
	std::set<Item*> _playerInventory;
	std::map<ItemSource*, Item*> _pendingItemSources;
	std::vector<WorldPath*> _pendingPaths;
	std::vector<Item*> _fillerItems;

	std::set<Item*> _strictlyNeededKeyItems;
};
