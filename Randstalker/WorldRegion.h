#pragma once

#include <set>

class WorldRegion;

class WorldPath
{
public:
    WorldPath(WorldRegion* destination, Item* requiredItem = nullptr) :
        _destination(destination),
        _requiredItem(requiredItem)
    {}

    WorldRegion* getDestination() const { return _destination; }
    Item* getRequiredItem() const { return _requiredItem; }

private:
    WorldRegion* _destination;
    Item* _requiredItem;
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

    const std::vector<WorldPath*>& getOutgoingPaths() const { return _outgoingPaths; }

    const std::string& getName() const { return _name;  }

private:
    std::vector<AbstractItemSource*> _itemSources;
    std::vector<WorldPath*> _outgoingPaths;
    std::string _name;
};