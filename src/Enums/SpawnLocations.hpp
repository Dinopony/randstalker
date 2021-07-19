#pragma once

#include <string>
#include <vector>

enum class SpawnLocation {
    MASSAN,
    GUMI,
    RYUMA,
    RANDOM
};

std::string spawnLocationToString(SpawnLocation loc);
int spawnLocationToNumber(SpawnLocation loc);
SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue = SpawnLocation::MASSAN);
std::vector<SpawnLocation> getAllSpawnLocations();

uint8_t getSpawnLocationX(SpawnLocation loc);
uint8_t getSpawnLocationZ(SpawnLocation loc);
uint16_t getSpawnLocationMapID(SpawnLocation loc);
