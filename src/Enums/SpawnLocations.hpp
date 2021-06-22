#pragma once

#include <string>

enum class SpawnLocation {
    MASSAN,
    GUMI,
    RYUMA,
    RANDOM
};

std::string spawnLocationToString(SpawnLocation loc);
int spawnLocationToNumber(SpawnLocation loc);
SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue = SpawnLocation::MASSAN);