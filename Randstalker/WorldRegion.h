#pragma once

#include <set>

class WorldRegion;

class WorldPath
{
public:
    WorldPath(WorldRegion* destination, Item* requiredItem = nullptr, uint8_t requiredQuantity = 0) :
        _destination(destination),
        _requiredItem(requiredItem),
        _requiredQuantity(requiredQuantity)
    {}

    WorldRegion* getDestination() const { return _destination; }
    Item* getRequiredItem() const { return _requiredItem; }
    uint8_t getRequiredItemQuantity() const { return _requiredQuantity; }

private:
    WorldRegion* _destination;
    Item* _requiredItem;
    uint8_t _requiredQuantity;
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

    void addPathTo(WorldRegion* otherRegion, Item* requiredItem = nullptr, uint8_t requiredQuantity = 0)
    {
        _outgoingPaths.push_back(new WorldPath(otherRegion, requiredItem, requiredQuantity));
    }

    void evaluateReachableRegions(const std::vector<Item*>& playerInventory, std::set<WorldRegion*>& reachableRegions, std::set<Item*>& keyItemsToProgress)
    {
        reachableRegions.insert(this);

        for(auto it = _outgoingPaths.begin() ; it != _outgoingPaths.end() ; ++it)
        {
			WorldPath* worldPath = *it;

            WorldRegion* destination = worldPath->getDestination();
            if(reachableRegions.count(destination))
                continue;

            Item* requiredItem = worldPath->getRequiredItem();
            if(requiredItem)
            {
                uint8_t foundItemsInInventory = 0;
				for(auto it2 = playerInventory.begin() ; it2 != playerInventory.end() ; ++it2)
				{
					Item* item = *it2;
                    if(item == requiredItem)
                        foundItemsInInventory++;
				}

                if(foundItemsInInventory == 0 || foundItemsInInventory < worldPath->getRequiredItemQuantity())
                {
                    keyItemsToProgress.insert(requiredItem);
                    continue;
                }
            }

            destination->evaluateReachableRegions(playerInventory, reachableRegions, keyItemsToProgress);
        }
    }

    const std::string& getName() const { return _name;  }

private:
    std::vector<AbstractItemSource*> _itemSources;
    std::vector<WorldPath*> _outgoingPaths;
    std::string _name;
};