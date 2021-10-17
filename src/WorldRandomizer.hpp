#pragma once

#include <vector>
#include <string>
#include <random>
#include "UnsortedSet.hpp"

#include "Enums/ItemCodes.hpp"
#include "Enums/ItemSourceCodes.hpp"
#include "Enums/RegionCodes.hpp"
#include "ItemSources.hpp"
#include "RandomizerOptions.hpp"
#include "Item.hpp"
#include "World.hpp"
#include "WorldRegion.hpp"

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
	UnsortedSet<Item*> analyzeStrictlyRequiredKeyItemsForRegion(WorldRegion* region);

	void placePriorityItems();
	void initFillerItems();

	void placeFillerItemsPhase(size_t count, Item* lastResortFiller = nullptr);
	void explorationPhase();
	void placeKeyItemsPhase();
	void unlockPhase();

	// Third pass randomizations (after items)
	void randomizeHints();
	Item* randomizeFortuneTellerHint();
	Item* randomizeOracleStoneHint(Item* forbiddenFortuneTellerItem);
	void randomizeSignHints(Item* hintedFortuneItem, Item* hintedOracleStoneItem);
	uint32_t getNextElligibleHintableItemPos(std::vector<uint8_t> hintableItemsNecessity, UnsortedSet<Item*> itemsAlreadyObtainedAtSign);
	
	std::string getRandomHintForItem(Item* item);
	std::string getRandomHintForItemSource(ItemSource* itemSource);

	void randomizeTiborTrees();

private:
	World& _world;
	const RandomizerOptions& _options;

	std::ofstream _debugLog;
	std::mt19937 _rng;

	UnsortedSet<WorldRegion*> _regionsToExplore;
	UnsortedSet<WorldRegion*> _exploredRegions;
	std::vector<ItemSource*> _itemSourcesToFill;
	std::vector<Item*> _playerInventory;
	std::vector<WorldPath*> _pendingPaths;
	std::vector<Item*> _fillerItems;

	UnsortedSet<Item*> _strictlyNeededKeyItems;
};
