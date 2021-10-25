#include "spawn_location.hpp"
#include "../exceptions.hpp"
#include <unordered_map>

const std::map<std::string, SpawnLocation>& getAllSpawnLocations()
{
    static std::map<std::string, SpawnLocation> allSpawnLocations;
    if(allSpawnLocations.empty())
    {
        allSpawnLocations["massan"]    = SpawnLocation("massan",    0x258, 0x1F, 0x19, ORIENTATION_SW, RegionCode::MASSAN, 4);
        allSpawnLocations["gumi"]      = SpawnLocation("gumi",      0x25E, 0x10, 0x0F, ORIENTATION_SW, RegionCode::GUMI, 4);
        allSpawnLocations["ryuma"]     = SpawnLocation("ryuma",     0x268, 0x11, 0x14, ORIENTATION_SE, RegionCode::RYUMA, 4);
        allSpawnLocations["waterfall"] = SpawnLocation("waterfall", 0x214, 0x38, 0x1A, ORIENTATION_SE, RegionCode::ROUTE_MASSAN_GUMI, 4);
        allSpawnLocations["kado"]      = SpawnLocation("kado",      0x1BA, 0x16, 0x12, ORIENTATION_NW, RegionCode::ROUTE_GUMI_RYUMA, 4);
        allSpawnLocations["mercator"]  = SpawnLocation("mercator",  0x2B3, 0x10, 0x0F, ORIENTATION_SW, RegionCode::MERCATOR, 10);
        allSpawnLocations["verla"]     = SpawnLocation("verla",     0x2D2, 0x16, 0x16, ORIENTATION_SW, RegionCode::VERLA, 16);
        allSpawnLocations["destel"]    = SpawnLocation("destel",    0x2DF, 0x11, 0x12, ORIENTATION_SW, RegionCode::DESTEL, 20);
        allSpawnLocations["greenmaze"] = SpawnLocation("greenmaze", 0x233, 0x1D, 0x21, ORIENTATION_NE, RegionCode::GREENMAZE_PRE_WHISTLE, 10);
    }

    return allSpawnLocations;
}

const SpawnLocation& getSpawnLocationFromName(const std::string& name)
{
    try {
        std::string lowercaseName = name;
        Tools::toLower(lowercaseName);
        return getAllSpawnLocations().at(lowercaseName);
    } 
    catch(std::out_of_range&)
    {
        std::string msg = std::string("Unknown spawn location '") + name + "'";
        throw RandomizerException(msg);
    }
}
