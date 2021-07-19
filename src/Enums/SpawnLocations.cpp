#include "SpawnLocations.hpp"
#include "../Tools.hpp"
#include <unordered_map>

std::string spawnLocationToString(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::RANDOM:		return "Random";
    case SpawnLocation::MASSAN:		return "Massan";
    case SpawnLocation::GUMI:		return "Gumi";
    case SpawnLocation::RYUMA:		return "Ryuma";
    case SpawnLocation::WATERFALL:	return "Near Waterfall Shrine";
    case SpawnLocation::KADO:		return "Kado's House";
    }
    return "???";
}

int spawnLocationToNumber(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::RANDOM:		return 0;
    case SpawnLocation::MASSAN:		return 1;
    case SpawnLocation::GUMI:		return 2;
    case SpawnLocation::RYUMA:		return 3;
    case SpawnLocation::WATERFALL:	return 4;
    case SpawnLocation::KADO:		return 5;
    }
    return -1;
}

SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue)
{
    const std::unordered_map<std::string, SpawnLocation> assocMap = {
        { "random",		SpawnLocation::RANDOM		},
        { "0",			SpawnLocation::RANDOM		},

        { "massan", 	SpawnLocation::MASSAN		},
        { "1",			SpawnLocation::MASSAN		},

        { "gumi",		SpawnLocation::GUMI			},
        { "2",			SpawnLocation::GUMI			},

        { "ryuma",		SpawnLocation::RYUMA		},
        { "3",			SpawnLocation::RYUMA		},

        { "waterfall",	SpawnLocation::WATERFALL	},
        { "4",			SpawnLocation::WATERFALL	},

        { "kado",		SpawnLocation::KADO			},
        { "5",			SpawnLocation::KADO			}
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

std::vector<SpawnLocation> getAllSpawnLocations()
{ 
    return { 
        SpawnLocation::MASSAN, 
        SpawnLocation::GUMI,
        SpawnLocation::RYUMA,
        SpawnLocation::WATERFALL, 
        SpawnLocation::KADO
    };
}

uint8_t getSpawnLocationX(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return 0x1F;
    case SpawnLocation::GUMI:		return 0x10;
    case SpawnLocation::RYUMA:		return 0x11;
    case SpawnLocation::WATERFALL:	return 0x38;
    case SpawnLocation::KADO:		return 0x16;
    }
    return -1;
}

uint8_t getSpawnLocationZ(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return 0x19;
    case SpawnLocation::GUMI:		return 0x0F;
    case SpawnLocation::RYUMA:		return 0x14;
    case SpawnLocation::WATERFALL:	return 0x1A;
    case SpawnLocation::KADO:		return 0x12;
    }
    return -1;
}

uint16_t getSpawnLocationMapID(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return 0x258;
    case SpawnLocation::GUMI:		return 0x25E;
    case SpawnLocation::RYUMA:		return 0x268;
    case SpawnLocation::WATERFALL:	return 0x214;
    case SpawnLocation::KADO:		return 0x1BA;
    }
    return -1;
}

uint8_t getSpawnLocationOrientation(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:		return SPAWN_ORIENTATION_SW;
    case SpawnLocation::GUMI:		return SPAWN_ORIENTATION_SW;
    case SpawnLocation::RYUMA:		return SPAWN_ORIENTATION_SW;
    case SpawnLocation::WATERFALL:	return SPAWN_ORIENTATION_SE;
    case SpawnLocation::KADO:		return SPAWN_ORIENTATION_NW;
    }
    return -1;
}