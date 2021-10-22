#pragma once

#include "WorldPath.hpp"
#include "WorldRegion.hpp"
#include "Exceptions.hpp"

class ExplorationPath
{
	UnsortedSet<WorldPath*> _paths;
	UnsortedSet<WorldRegion*> _exploredRegions;
	UnsortedSet<Item*> _requiredItems;
    bool _invalid;

public:
	ExplorationPath(WorldRegion* startingRegion, const UnsortedSet<Item*>& requiredItems = {}) : 
        _paths(),
        _exploredRegions({ startingRegion }),
        _requiredItems(requiredItems),
        _invalid(false)
	{}

	ExplorationPath fork(WorldPath* path) const
	{
		ExplorationPath newExploration = *this;
		newExploration._paths.insert(path);
		newExploration._exploredRegions.insert(path->getOrigin());

		const std::vector<Item*>& requiredItems = path->getRequiredItems();
		for(Item* item : requiredItems)
			newExploration._requiredItems.insert(item);

		return newExploration;
	}

    void merge(const ExplorationPath& path)
    {
        _paths.insert(path._paths);
        _exploredRegions.insert(path._exploredRegions);
        _requiredItems.insert(path._requiredItems);
        if(path._invalid)
            throw RandomizerException("Trying to merge an invalid path");
    }

    bool isInvalid() const { return _invalid; }
    void invalidate() { _invalid = true; }

	bool hasRegionAlreadyBeenExplored(WorldRegion* region) const { return _exploredRegions.contains(region); }
	WorldRegion* getLatestRegion() const { return *_exploredRegions.rbegin(); }
    const UnsortedSet<WorldRegion*>& getExploredRegions() const { return _exploredRegions; }
	const UnsortedSet<Item*>& getRequiredItems() const { return _requiredItems; }
	uint32_t getTotalCost() const { return (uint32_t)_requiredItems.size(); }
};