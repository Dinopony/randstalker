#pragma once

#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <random>

#include "Constants/ItemCodes.h"
#include "Constants/ItemSourceCodes.h"
#include "Constants/RegionCodes.h"

#include "Item.h"
#include "ItemSource.h"
#include "World.h"
#include "WorldRegion.h"

class NoAppropriateItemSourceException : public std::exception {};

class WorldRandomizer
{
public:
	WorldRandomizer(World& world, uint32_t seed, std::ofstream& logFile, const std::map<std::string, std::string>& options);
	~WorldRandomizer() {}

	void randomize();
	void randomizeSpawnPoint();
	void randomizeItems();

private:
	void initPriorityItems();
	void initFillerItems();

	std::vector<WorldRegion*> evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources);
	
	void fillSourcesWithPriorityItems();
	void fillSourcesWithFillerItems(std::vector<AbstractItemSource*>::iterator begin, std::vector<AbstractItemSource*>::iterator end);

	void writeItemSourcesBreakdownInLog();

	void randomizeTiborTrees();

private:
	World& _world;

	std::ofstream& _logFile;
	std::mt19937 _rng;

	bool _shuffleTiborTrees;
	bool _randomSpawnPoint;

	std::vector<Item*> _priorityItems;
	std::vector<Item*> _fillerItems;
};
