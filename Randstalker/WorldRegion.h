#pragma once

#include <vector>

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

	void addRequiredItem(Item* requiredItem) { _requiredItems.push_back(requiredItem); }

	WorldRegion* getDestination() const { return _destination; }
	const std::vector<Item*> getRequiredItems() const { return _requiredItems; }

private:
	WorldRegion* _destination;
	std::vector<Item*> _requiredItems;
	std::vector<uint16_t> _darkRooms;
};

//////////////////////////////////////////////////////////////////////////////////////////////

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

	void addPathTo(WorldRegion* otherRegion, Item* requiredItem = nullptr) 
	{
		WorldPath* newPath = new WorldPath(otherRegion, requiredItem);
		_outgoingPaths.push_back(newPath);
		otherRegion->_ingoingPaths.push_back(newPath);
	}

	void addPathTo(WorldRegion* otherRegion, const std::vector<Item*>& requiredItems) 
	{
		WorldPath* newPath = new WorldPath(otherRegion, requiredItems);
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
	std::vector<AbstractItemSource*> _itemSources;
	std::vector<std::pair<AbstractItemSource*, Item*>> _restrictedItemSources;

	std::vector<WorldPath*> _ingoingPaths;
	std::vector<WorldPath*> _outgoingPaths;
	
	std::vector<std::string> _hints;
	std::vector<uint16_t> _darkRooms;
};