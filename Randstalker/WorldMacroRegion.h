#pragma once

#include <string>
#include <vector>
#include <set>
#include "Item.h"
#include "ItemSources.h"
#include "WorldRegion.h"

class WorldMacroRegion
{
public:
	WorldMacroRegion(const std::string& name, const std::vector<WorldRegion*> regions) :
		_name(name),
		_regions(regions)
	{}

	const std::string& getName() const { return _name; }
	const std::vector<WorldRegion*>& getRegions() const { return _regions; }

	bool isBarren(const std::set<Item*> strictlyNeededKeyItems)
	{
		for (WorldRegion* region : _regions)
		{
			std::vector<ItemSource*> itemSources = region->getItemSources();
			for (ItemSource* source : itemSources)
			{
				if (strictlyNeededKeyItems.count(source->getItem()))
					return false;
			}
		}
		return true;
	}

private:
	std::string _name;
	std::vector<WorldRegion*> _regions;
};