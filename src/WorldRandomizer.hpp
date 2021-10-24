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

	void randomize();

	Json getPlaythroughAsJson() const;
	const Json& getDebugLogAsJson() const { return _debugLogJson; }

private:
	void initFillerItems();
	void initMandatoryItems();
	void initInventoryWithStartingItems();
	
	// First pass randomizations (before items)
	void randomizeSpawnLocation();
	void randomizeGoldValues();
	void randomizeDarkRooms();

	// Second pass randomizations (items)
	void placeMandatoryItems();
	void randomizeItems();

	void placeFillerItemsPhase(Json& debugLogStepJson, size_t count, Item* lastResortFiller = nullptr);
	void explorationPhase(Json& debugLogStepJson);
	void placeKeyItemsPhase(Json& debugLogStepJson);
	void unlockPhase();

	// Third pass randomizations (after items)
	void randomizeHints();
	Item* randomizeFortuneTellerHint();
	Item* randomizeOracleStoneHint(Item* forbiddenFortuneTellerItem);
	void randomizeSignHints(Item* hintedFortuneItem, Item* hintedOracleStoneItem);
	uint32_t getNextEligibleHintableItemPos(std::vector<uint8_t> hintableItemsNecessity, const std::vector<Item*>& itemsAlreadyObtainedAtSign);
	
	std::string getRandomHintForItem(Item* item);
	std::string getRandomHintForItemSource(ItemSource* itemSource);

	void randomizeTiborTrees();

private:
	World& _world;
	const RandomizerOptions& _options;

	Json _debugLogJson;
	std::mt19937 _rng;

	std::vector<Item*> _fillerItems;
	std::vector<Item*> _mandatoryItems;
	uint8_t _goldItemsCount;

	UnsortedSet<WorldRegion*> _regionsToExplore;
	UnsortedSet<WorldRegion*> _exploredRegions;
	std::vector<ItemSource*> _itemSourcesToFill;
	std::vector<Item*> _playerInventory;
	std::vector<WorldPath*> _pendingPaths;

	std::vector<ItemSource*> _logicalPlaythrough;

	std::vector<Item*> _minimalItemsToComplete;
};
