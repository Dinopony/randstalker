#pragma once

#include <set>

class WorldRegion;

class WorldPath
{
public:
	WorldPath(WorldRegion* destination, Item* requiredItem = nullptr) :
		_destination(destination),
		_requiredItems()
	{
		if (requiredItem)
			_requiredItems.push_back(requiredItem);
	}

	WorldPath(WorldRegion* destination, std::vector<Item*> requiredItems) :
		_destination(destination),
		_requiredItems(requiredItems)
	{}

	WorldRegion* getDestination() const { return _destination; }
	const std::vector<Item*> getRequiredItems() const { return _requiredItems; }

private:
	WorldRegion* _destination;
	std::vector<Item*> _requiredItems;
};

class WorldRegion
{
public:
	WorldRegion(const std::string& name) :
		_name(name)
	{}

	~WorldRegion() 
	{
		for (WorldPath* path : _outgoingPaths)
			delete path;
	}

	void addItemSource(AbstractItemSource* itemSource) { _itemSources.push_back(itemSource); }
	const std::vector<AbstractItemSource*>& getItemSources() const { return _itemSources; }

	void addPathTo(WorldRegion* otherRegion, Item* requiredItem = nullptr)
	{
		_outgoingPaths.push_back(new WorldPath(otherRegion, requiredItem));
	}

	void addPathTo(WorldRegion* otherRegion, const std::vector<Item*>& requiredItems)
	{
		_outgoingPaths.push_back(new WorldPath(otherRegion, requiredItems));
	}

	const std::vector<WorldPath*>& getOutgoingPaths() const { return _outgoingPaths; }

	const std::string& getName() const { return _name;  }

private:
	std::vector<AbstractItemSource*> _itemSources;
	std::vector<WorldPath*> _outgoingPaths;
	std::string _name;
};