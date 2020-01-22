#pragma once

#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <random>
#include <climits>

#include "Constants/ItemCodes.h"
#include "Constants/ItemPedestalCodes.h"
#include "Constants/ItemRewardCodes.h"

#include "Item.h"
#include "ItemSource.h"
#include "WorldRegion.h"
#include "OutsideTreeMap.h"

class NoAppropriateItemSourceException : public std::exception {};

class World 
{
public:
	World(uint32_t seed, std::ofstream& logFile);
	~World();

	void initItems();
	void initItemSources();
	void initRegions();
	void initTiborTrees();
	void initFillerItems();

	void randomize();
	std::vector<WorldRegion*> evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources);
	void fillSourcesWithFillerItems(const std::vector<AbstractItemSource*>& itemSources, uint32_t count = UINT_MAX);

	void shuffleTiborTrees();

	void writeToROM(GameROM& rom);
	void writeItemSourcesBreakdownInLog();
	void writeTiborJunctionsInLog();

private:
	std::ofstream& _logFile;
	std::mt19937 _rng;

	std::map<uint8_t, Item*> _items;
	std::map<uint8_t, ItemChest*> _chests;
	std::map<ItemPedestalCode, ItemPedestal*> _pedestals;
	std::map<ItemRewardCode, ItemReward*> _rewards;

	std::vector<WorldRegion*> _regions;
	WorldRegion* _spawnRegion;

	std::vector<OutsideTreeMap*> _outsideTreeMaps;

	std::vector<Item*> _fillerItems;
};
