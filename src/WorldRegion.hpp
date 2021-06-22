#pragma once

#include <vector>
#include "UnsortedSet.hpp"

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

	bool canBeCrossedWithInventory(const UnsortedSet<Item*>& playerInventory)
	{
		for (Item* requiredItem : _requiredItems)
			if (!playerInventory.contains(requiredItem))
				return false;
		return true;
	}

	void setRandomWeight(uint16_t randomWeight) { _randomWeight = randomWeight; }
	uint16_t getRandomWeight() const { return _randomWeight; }

private:
	WorldRegion* _origin;
	WorldRegion* _destination;
	std::vector<Item*> _requiredItems;
	std::vector<uint16_t> _darkRooms;
	uint16_t _randomWeight;
};

//////////////////////////////////////////////////////////////////////////////////////////////

class WorldRegion
{
public:
	WorldRegion(const std::string& name, const std::vector<ItemSource*> itemSources) :
		_name		(name),
		_itemSources(itemSources)
	{
		for (ItemSource* source : itemSources)
			source->setRegion(this);
	}

	~WorldRegion() 
	{
		for (WorldPath* path : _outgoingPaths)
			delete path;
	}

	const std::string& getName() const { return _name; }

	std::vector<ItemSource*> getItemSources() const
	{
		return _itemSources;
	}

	void addItemSource(ItemSource* itemSource)
	{
		_itemSources.push_back(itemSource);
		itemSource->setRegion(this);
	}

	void addPathTo(WorldRegion* otherRegion, Item* requiredItem = nullptr, uint16_t weight = 1)
	{
		WorldPath* newPath = new WorldPath(this, otherRegion, requiredItem, weight);
		_outgoingPaths.push_back(newPath);
		otherRegion->_ingoingPaths.push_back(newPath);
	}

	void addPathTo(WorldRegion* otherRegion, const std::vector<Item*>& requiredItems, uint16_t weight = 1)
	{
		WorldPath* newPath = new WorldPath(this, otherRegion, requiredItems, weight);
		_outgoingPaths.push_back(newPath);
		otherRegion->_ingoingPaths.push_back(newPath);
	}

	const std::vector<WorldPath*>& getIngoingPaths() const { return _ingoingPaths; }
	const std::vector<WorldPath*>& getOutgoingPaths() const { return _outgoingPaths; }

	void addHint(const std::string& hint) { _hints.push_back(hint); }
	const std::vector<std::string>& getHints() const { return _hints; }

	void setDarkRooms(const std::vector<uint16_t>& darkRooms) { _darkRooms = darkRooms; }
	void setDarkRooms(uint16_t lowerBound, uint16_t upperBound)
	{ 
		_darkRooms.clear();
		for (uint16_t i = lowerBound; i <= upperBound; ++i)
			_darkRooms.push_back(i);
	}
	const std::vector<uint16_t>& getDarkRooms() const { return _darkRooms; }

private:
	std::string _name;
	std::vector<ItemSource*> _itemSources;

	std::vector<WorldPath*> _ingoingPaths;
	std::vector<WorldPath*> _outgoingPaths;
	
	std::vector<std::string> _hints;
	std::vector<uint16_t> _darkRooms;
};