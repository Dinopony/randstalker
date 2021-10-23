#include "SpawnLocations.hpp"
#include "../Tools.hpp"
#include <unordered_map>

std::string spawnLocationToString(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::RANDOM:      return "Random";
    case SpawnLocation::MASSAN:      return "Massan";
    case SpawnLocation::GUMI:        return "Gumi";
    case SpawnLocation::RYUMA:       return "Ryuma";
    case SpawnLocation::WATERFALL:   return "Near Waterfall Shrine";
    case SpawnLocation::KADO:        return "Kado's House";
    case SpawnLocation::MERCATOR:    return "Mercator";
    case SpawnLocation::VERLA:       return "Verla";
    case SpawnLocation::DESTEL:      return "Destel";
    case SpawnLocation::GREENMAZE:   return "Greenmaze";
    }
    return "???";
}

int spawnLocationToNumber(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::RANDOM:       return 0;
    case SpawnLocation::MASSAN:       return 1;
    case SpawnLocation::GUMI:         return 2;
    case SpawnLocation::RYUMA:        return 3;
    case SpawnLocation::WATERFALL:    return 4;
    case SpawnLocation::KADO:         return 5;
    case SpawnLocation::MERCATOR:     return 6;
    case SpawnLocation::VERLA:        return 7;
    case SpawnLocation::DESTEL:       return 8;
    case SpawnLocation::GREENMAZE:    return 9;
    }
    return -1;
}

SpawnLocation spawnLocationFromString(std::string locAsString)
{
    const std::unordered_map<std::string, SpawnLocation> assocMap = {
        { "random",         SpawnLocation::RANDOM        },
        { "0",              SpawnLocation::RANDOM        },

        { "massan",         SpawnLocation::MASSAN        },
        { "1",              SpawnLocation::MASSAN        },

        { "gumi",           SpawnLocation::GUMI            },
        { "2",              SpawnLocation::GUMI            },

        { "ryuma",          SpawnLocation::RYUMA        },
        { "3",              SpawnLocation::RYUMA        },

        { "waterfall",      SpawnLocation::WATERFALL    },
        { "4",              SpawnLocation::WATERFALL    },

        { "kado",           SpawnLocation::KADO            },
        { "5",              SpawnLocation::KADO            },

        { "mercator",       SpawnLocation::MERCATOR        },
        { "6",              SpawnLocation::MERCATOR        },
        
        { "verla",          SpawnLocation::VERLA        },
        { "7",              SpawnLocation::VERLA        },
        
        { "destel",         SpawnLocation::DESTEL        },
        { "8",              SpawnLocation::DESTEL        },
        
        { "greenmaze",      SpawnLocation::GREENMAZE    },
        { "9",              SpawnLocation::DESTEL        }
    };

    Tools::toLower(locAsString);
    return assocMap.at(locAsString);
}

SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue)
{
    try {
        return spawnLocationFromString(locAsString);
    } catch (std::out_of_range&) {
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
        SpawnLocation::KADO,
        SpawnLocation::MERCATOR,
        SpawnLocation::VERLA,
        SpawnLocation::DESTEL,
        SpawnLocation::GREENMAZE
    };
}

uint8_t getSpawnLocationX(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:       return 0x1F;
    case SpawnLocation::GUMI:         return 0x10;
    case SpawnLocation::RYUMA:        return 0x11;
    case SpawnLocation::WATERFALL:    return 0x38;
    case SpawnLocation::KADO:         return 0x16;
    case SpawnLocation::MERCATOR:     return 0x10;
    case SpawnLocation::VERLA:        return 0x16;
    case SpawnLocation::DESTEL:       return 0x11;
    case SpawnLocation::GREENMAZE:    return 0x1D;
    }
    return -1;
}

uint8_t getSpawnLocationZ(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:       return 0x19;
    case SpawnLocation::GUMI:         return 0x0F;
    case SpawnLocation::RYUMA:        return 0x14;
    case SpawnLocation::WATERFALL:    return 0x1A;
    case SpawnLocation::KADO:         return 0x12;
    case SpawnLocation::MERCATOR:     return 0x0F;
    case SpawnLocation::DESTEL:       return 0x12;
    case SpawnLocation::GREENMAZE:    return 0x21;
    }
    return -1;
}

uint16_t getSpawnLocationMapID(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:       return 0x258;
    case SpawnLocation::GUMI:         return 0x25E;
    case SpawnLocation::RYUMA:        return 0x268;
    case SpawnLocation::WATERFALL:    return 0x214;
    case SpawnLocation::KADO:         return 0x1BA;
    case SpawnLocation::MERCATOR:     return 0x2B3;
    case SpawnLocation::VERLA:        return 0x2D2;
    case SpawnLocation::DESTEL:       return 0x2DF;
    case SpawnLocation::GREENMAZE:    return 0x233;
    }
    return -1;
}

uint8_t getSpawnLocationOrientation(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:       return SPAWN_ORIENTATION_SW;
    case SpawnLocation::GUMI:         return SPAWN_ORIENTATION_SW;
    case SpawnLocation::RYUMA:        return SPAWN_ORIENTATION_SW;
    case SpawnLocation::WATERFALL:    return SPAWN_ORIENTATION_SE;
    case SpawnLocation::KADO:         return SPAWN_ORIENTATION_NW;
    case SpawnLocation::MERCATOR:     return SPAWN_ORIENTATION_SW;
    case SpawnLocation::VERLA:        return SPAWN_ORIENTATION_SW;
    case SpawnLocation::DESTEL:       return SPAWN_ORIENTATION_SW;
    case SpawnLocation::GREENMAZE:    return SPAWN_ORIENTATION_NE;
    }
    return -1;
}

RegionCode getSpawnLocationRegion(SpawnLocation loc)
{
    switch (loc)
    {
    case SpawnLocation::MASSAN:      return RegionCode::MASSAN;
    case SpawnLocation::GUMI:        return RegionCode::GUMI;
    case SpawnLocation::RYUMA:       return RegionCode::RYUMA;
    case SpawnLocation::WATERFALL:   return RegionCode::ROUTE_MASSAN_GUMI;
    case SpawnLocation::KADO:        return RegionCode::ROUTE_GUMI_RYUMA;
    case SpawnLocation::MERCATOR:    return RegionCode::MERCATOR;
    case SpawnLocation::VERLA:       return RegionCode::VERLA;
    case SpawnLocation::DESTEL:      return RegionCode::DESTEL;
    case SpawnLocation::GREENMAZE:   return RegionCode::GREENMAZE_PRE_WHISTLE;
    }
    return RegionCode::ENDGAME;
}