#pragma once
#include "Item.hpp"

class WorldRegion;

class WorldPath
{
public:
	WorldPath(WorldRegion* origin, WorldRegion* destination, Item* requiredItem = nullptr, uint16_t weight = 1) :
		_origin(origin),
		_destination(destination),
		_randomWeight(weight),
		_requiredItems()
	{
		if (requiredItem)
			_requiredItems.push_back(requiredItem);
	}

	WorldPath(WorldRegion* origin, WorldRegion* destination, std::vector<Item*> requiredItems, uint16_t weight = 1) :
		_origin(origin),
		_destination(destination),
		_randomWeight(weight),
		_requiredItems(requiredItems)
	{}

	void addRequiredItem(Item* requiredItem) { _requiredItems.push_back(requiredItem); }

	WorldRegion* getOrigin() const { return _origin; }
	WorldRegion* getDestination() const { return _destination; }
	const std::vector<Item*>& getRequiredItems() const { return _requiredItems; }

	std::vector<Item*> getMissingItemsToCross(std::vector<Item*> playerInventoryCopy, bool onlyHardRequiredItems = false)
	{
		std::vector<Item*> itemsToTest = _requiredItems;
		if(!onlyHardRequiredItems)
		{
			for(Item* item : _nonRequiredItemsPlacedWhenCrossing)
				itemsToTest.push_back(item);
		}

		std::vector<Item*> missingItems;
		for (Item* item : itemsToTest)
		{
			auto iter = std::find(playerInventoryCopy.begin(), playerInventoryCopy.end(), item);
			if (iter == playerInventoryCopy.end())
			{
				missingItems.push_back(item);
			}
			else
			{
				// If item has been found, remove it from the inventory copy for it not to count several times
				// (case where quantity needed is > 1)
				playerInventoryCopy.erase(iter);
			}
		}
	
		return missingItems;
	}

	void addNonRequiredItemPlacedWhenCrossing(Item* item)
	{
		_nonRequiredItemsPlacedWhenCrossing.push_back(item);
	}

	const std::vector<Item*>& getNonRequiredItemsPlacedWhenCrossing() const
	{
		return _nonRequiredItemsPlacedWhenCrossing;
	}

	void setRandomWeight(uint16_t randomWeight) { _randomWeight = randomWeight; }
	uint16_t getRandomWeight() const { return _randomWeight; }

private:
	WorldRegion* _origin;
	WorldRegion* _destination;
	std::vector<Item*> _requiredItems;
	std::vector<Item*> _nonRequiredItemsPlacedWhenCrossing;
	std::vector<uint16_t> _darkRooms;
	uint16_t _randomWeight;
};