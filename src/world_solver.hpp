#pragma once

#include "model/world_region.hpp"
#include "model/item_source.hpp"

class WorldSolver
{
public:
    WorldSolver(WorldRegion* startNode, WorldRegion* endNode) :
        _startNode(startNode),
        _endNode(endNode),
        _exploredRegions({ _startNode }),
        _pathsToProcess(_startNode->outgoing_paths())
    {}

    void forbidItems(const UnsortedSet<Item*>& forbiddenItems)
    { 
        _forbiddenItems = forbiddenItems;
    }

    void forbidTakingItemsFromRegions(const UnsortedSet<WorldRegion*>& forbiddenRegionsToPickItems)
    { 
        _forbiddenRegionsToPickItems = forbiddenRegionsToPickItems;
    }

    bool tryToSolve()
    {
        while(true)
        {
            this->expandExplorationZone();

            if(_exploredRegions.contains(_endNode))
                return true;

            bool didntSolveAnyPath = true;
            for(WorldPath* blockedPath : _blockedPaths)
            {
                std::vector<Item*> missingItems = blockedPath->missing_items_to_cross(_inventory, true);
                std::vector<ItemSource*> itemSourcesToPick;
                bool allLocated = true;
                for(Item* item : missingItems)
                {
                    std::vector<ItemSource*> validReachableSources;
                    if(!_forbiddenItems.contains(item))
                        validReachableSources = this->getReachableItemSources(item);
                     
                    if(validReachableSources.empty())
                    {
                        allLocated = false;
                        break;                
                    }
        
                    itemSourcesToPick.push_back(*validReachableSources.begin());
                }

                if(allLocated)
                {
                    // Path can be taken, initiate a sub-exploration by forking exploration state
                    for (ItemSource* source : itemSourcesToPick)
                        this->pickItemSource(source);

                    this->explorePath(blockedPath);
                    didntSolveAnyPath = false;
                    break;
                }
            }

            if(didntSolveAnyPath)
                return false;
        }
    }

    void expandExplorationZone()
    {
        while(!_pathsToProcess.empty())
        {
            WorldPath* firstPath = *_pathsToProcess.begin();
            _pathsToProcess.erase(_pathsToProcess.begin());

            // Region at the end of this path has already been explored, it's useless to evaluate it
            if(_exploredRegions.contains(firstPath->destination()))
                continue;

            std::vector<Item*> missingItems = firstPath->missing_items_to_cross(_inventory, true);
            if(missingItems.empty())
            {
                // If we have already have the items to cross this path, explore it to expand our reach
                this->explorePath(firstPath);
            }
            else
            {
                // Otherwise, add it to the list of blocked paths that will need to be processed later
                _blockedPaths.insert(firstPath);
            }
        }
    }

    void explorePath(WorldPath* path)
    {
        // In case path was a blocking one, remove it from blocked paths
        _blockedPaths.erase(path);

        WorldRegion* destination = path->destination();
        _exploredRegions.insert(destination);
        const std::vector<WorldPath*> outgoingPaths = destination->outgoing_paths();
        _pathsToProcess.insert(_pathsToProcess.end(), outgoingPaths.begin(), outgoingPaths.end());
    }

    void pickItemSource(ItemSource* source)
    {
        _pickedItemSources.insert(source);
        _inventory.push_back(source->item());
    }

    std::vector<ItemSource*> getReachableItemSources(Item* itemConstraint = nullptr)
    {
        std::vector<ItemSource*> reachableSources;
        for(WorldRegion* region : _exploredRegions)
        {
            // If picking items in this region is strictly forbidden, don't expose its item sources
            if(_forbiddenRegionsToPickItems.contains(region))
                continue;

            for(ItemSource* source : region->item_sources())
            {
                if(_pickedItemSources.contains(source))
                    continue;

                if(itemConstraint && source->item() != itemConstraint)
                    continue;

                reachableSources.push_back(source);
            }
        }

        return reachableSources;
    }

    const std::vector<Item*>& getInventory() const { return _inventory; }

private:
    WorldRegion* _startNode;
    WorldRegion* _endNode;
    UnsortedSet<Item*> _forbiddenItems;
    UnsortedSet<WorldRegion*> _forbiddenRegionsToPickItems;

    std::vector<Item*> _inventory;

    UnsortedSet<WorldRegion*> _exploredRegions;
    std::vector<WorldPath*> _pathsToProcess;
    UnsortedSet<WorldPath*> _blockedPaths;
    UnsortedSet<ItemSource*> _pickedItemSources;
};