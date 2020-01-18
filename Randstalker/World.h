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

class InfiniteLoopException : public std::exception {};

class World 
{
public:
	World()
	{
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

	void randomize(uint32_t seed, std::ofstream& logFile);

	void writeToROM(GameROM& rom)
	{
		for (auto& [key, item] : _items)
			item->writeToROM(rom);
		for (auto& [key, chest] : _chests)
			chest->writeToROM(rom);
		for (auto& [key, pedestal] : _pedestals)
			pedestal->writeToROM(rom);
		for (auto& [key, reward] : _rewards)
			reward->writeToROM(rom);
	}

	void writeToLog(std::ofstream& file)
	{
		for (WorldRegion* region : _regions)
		{
			auto sources = region->getItemSources();
			if (sources.empty())
				continue;

			file << "\n-------------------------------\n";
			file << "\t" << region->getName() << "\n\n";

			for (AbstractItemSource* source : sources)
			{
				file << (source->getItem() ? source->getItem()->getName() : "No item") << " in \"" << source->getName() << "\"\n";
			}
		}

		file << "\n-------------------------------\n";
		file << "Unplaced items:" << "\n";

		for (Item* item : _fillerItems)
			file << "- " << item->getName() << "\n";
	}

private:
	std::map<uint8_t, Item*> _items;
	std::map<uint8_t, ItemChest*> _chests;
	std::map<ItemPedestalCode, ItemPedestal*> _pedestals;
	std::map<ItemRewardCode, ItemReward*> _rewards;

	std::vector<WorldRegion*> _regions;
	WorldRegion* _spawnRegion;

	std::vector<Item*> _fillerItems;
};

























