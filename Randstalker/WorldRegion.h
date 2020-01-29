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
	WorldRegion(const std::string& name, const std::vector<AbstractItemSource*> itemSources) :
		_name		(name),
		_itemSources(itemSources)
	{}

	~WorldRegion() 
	{
		for (WorldPath* path : _outgoingPaths)
			delete path;
	}

	const std::string& getName() const { return _name; }

	void addItemSource(AbstractItemSource* itemSource) 				{ _itemSources.push_back(itemSource); }
	const std::vector<AbstractItemSource*>& getItemSources() const 	{ return _itemSources; }

	void addItemSource(AbstractItemSource* itemSource, Item* requiredItem) { _restrictedItemSources.push_back(std::make_pair(itemSource, requiredItem)); }
	const std::vector<std::pair<AbstractItemSource*, Item*>>& getRestrictedItemSources() const { return _restrictedItemSources; }

	std::vector<AbstractItemSource*> getAllItemSources() const 
	{ 
		std::vector<AbstractItemSource*> allItemSources = _itemSources;
		for (auto& [source, item] : _restrictedItemSources)
			allItemSources.push_back(source);
		return allItemSources;
	}

	void addPathTo(WorldRegion* otherRegion, Item* requiredItem = nullptr) {
		_outgoingPaths.push_back(new WorldPath(otherRegion, requiredItem));
	}
	void addPathTo(WorldRegion* otherRegion, const std::vector<Item*>& requiredItems) {
		_outgoingPaths.push_back(new WorldPath(otherRegion, requiredItems));
	}

	const std::vector<WorldPath*>& getOutgoingPaths() const { return _outgoingPaths; }

private:
	std::string _name;
	std::vector<AbstractItemSource*> _itemSources;
	std::vector<std::pair<AbstractItemSource*, Item*>> _restrictedItemSources;

	std::vector<WorldPath*> _outgoingPaths;

};