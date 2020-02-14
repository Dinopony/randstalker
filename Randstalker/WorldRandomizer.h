#pragma once

#include <vector>
#include <string>
#include <random>

#include "Constants/ItemCodes.h"
#include "Constants/ItemSourceCodes.h"
#include "Constants/RegionCodes.h"

#include "RandomizerOptions.h"
#include "Item.h"
#include "ItemSource.h"
#include "World.h"
#include "WorldRegion.h"

class NoAppropriateItemSourceException : public std::exception {};

class WorldRandomizer
{
public:
	WorldRandomizer(World& world, const RandomizerOptions& options);
	~WorldRandomizer() {}

	void randomize();

private:
	void setSpawnPoint();
	void randomizeGoldValues();
	void initPriorityItems();
	void initFillerItems();

	void randomizeItems();

	std::vector<WorldRegion*> evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources);
	
	void fillSourcesWithPriorityItems();
	void fillSourcesWithFillerItems(std::vector<AbstractItemSource*>::iterator begin, std::vector<AbstractItemSource*>::iterator end);

	void writeItemSourcesBreakdownInLog();

	void randomizeTiborTrees();

private:
	World& _world;
	const RandomizerOptions& _options;

	std::ofstream _logFile;
	std::mt19937 _rng;

	std::vector<Item*> _priorityItems;
	std::vector<Item*> _fillerItems;
};
