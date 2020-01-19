#pragma once

#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <random>

#include "Constants/ItemCodes.h"
#include "Constants/ItemPedestalCodes.h"
#include "Constants/ItemRewardCodes.h"

#include "Item.h"
#include "ItemSource.h"
#include "WorldRegion.h"
#include "Tools.h"

class NoAppropriateItemSourceException : public std::exception {};

class World 
{
public:
	World(uint32_t seed, std::ofstream& logFile) :
		_rng(seed),
		_logFile(logFile)
	{
		_logFile << "Seed: " << seed << "\n\n";

		this->initItems();
		this->initItemSources();
		this->initRegions();
		this->initFillerItems();
	}

	~World()
	{
		for (auto& [key, chest] : _chests)
			delete chest;
		for (auto& [key, pedestal] : _pedestals)
			delete pedestal;
		for (auto& [key, item] : _items)
			delete item;
		for (auto& [key, reward] : _rewards)
			delete reward;
		for (auto region : _regions)
			delete region;
	}

	void initItems();
	void initItemSources();
	void initRegions();
	void initFillerItems();

	void randomize();
	std::set<WorldRegion*> evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::vector<Item*>& out_keyItems, std::vector<AbstractItemSource*>& out_reachableSources);
	void fillSourcesWithFillerItems(const std::vector<AbstractItemSource*>& itemSources, uint32_t count = INT_MAX);

	void writeToROM(GameROM& rom);
	void writeItemSourcesBreakdownInLog();

private:
	std::ofstream& _logFile;
	std::mt19937 _rng;

	std::map<uint8_t, Item*> _items;
	std::map<uint8_t, ItemChest*> _chests;
	std::map<ItemPedestalCode, ItemPedestal*> _pedestals;
	std::map<ItemRewardCode, ItemReward*> _rewards;

	std::vector<WorldRegion*> _regions;
	WorldRegion* _spawnRegion;

	std::vector<Item*> _fillerItems;
};
