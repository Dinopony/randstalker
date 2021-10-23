#pragma once

#include <string>
#include <vector>
#include "UnsortedSet.hpp"
#include "Item.hpp"
#include "ItemSources.hpp"
#include "WorldRegion.hpp"

class WorldMacroRegion
{
public:
	WorldMacroRegion(const std::string& name, const std::vector<WorldRegion*> regions) :
		_name(name),
		_regions(regions)
	{}

	const std::string& getName() const { return _name; }
	const std::vector<WorldRegion*>& getRegions() const { return _regions; }

private:
	std::string _name;
	std::vector<WorldRegion*> _regions;
};