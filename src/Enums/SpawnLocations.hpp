#pragma once

#include <string>
#include <vector>

#define SPAWN_ORIENTATION_NE 0x08
#define SPAWN_ORIENTATION_SE 0x48
#define SPAWN_ORIENTATION_SW 0x88
#define SPAWN_ORIENTATION_NW 0xC8

enum class SpawnLocation {
    MASSAN,
    GUMI,
    RYUMA,
    WATERFALL,
    KADO,
    RANDOM
};

std::string spawnLocationToString(SpawnLocation loc);
int spawnLocationToNumber(SpawnLocation loc);
SpawnLocation spawnLocationFromString(std::string locAsString, SpawnLocation defaultValue = SpawnLocation::RANDOM);
std::vector<SpawnLocation> getAllSpawnLocations();

uint8_t getSpawnLocationX(SpawnLocation loc);
uint8_t getSpawnLocationZ(SpawnLocation loc);
uint16_t getSpawnLocationMapID(SpawnLocation loc);
uint8_t getSpawnLocationOrientation(SpawnLocation loc);