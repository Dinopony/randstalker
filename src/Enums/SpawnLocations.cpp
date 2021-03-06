#include "SpawnLocations.hpp"
#include "../Tools.hpp"
#include <unordered_map>

std::string spawnLocationToString(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return "Massan";
    case SpawnLocation::GUMI:		return "Gumi";
    case SpawnLocation::RYUMA:		return "Ryuma";
    case SpawnLocation::RANDOM:		return "Random";
    }
    return "???";
}

int spawnLocationToNumber(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return 0;
    case SpawnLocation::GUMI:		return 1;
    case SpawnLocation::RYUMA:		return 2;
    case SpawnLocation::RANDOM:		return 3;
    }
    return -1;
}

SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue)
{
	const std::unordered_map<std::string, SpawnLocation> assocMap = {
		{ "massan", SpawnLocation::MASSAN	},
		{ "0",		SpawnLocation::MASSAN	},
		{ "gumi",	SpawnLocation::GUMI		},
		{ "1",		SpawnLocation::GUMI		},
		{ "ryuma",	SpawnLocation::RYUMA	},
		{ "2",		SpawnLocation::RYUMA	},
		{ "random",	SpawnLocation::RANDOM	},
		{ "3",		SpawnLocation::RANDOM	}
	};

	try
	{
        Tools::toLower(locAsString);
		return assocMap.at(locAsString);
	}
	catch (std::out_of_range&)
	{
		return defaultValue;
	}
}