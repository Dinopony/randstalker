#pragma once

#include <string>
#include <vector>
#include "Enums/RegionCodes.hpp"
#include "Tools.hpp"

#define ORIENTATION_NE 0x08
#define ORIENTATION_SE 0x48
#define ORIENTATION_SW 0x88
#define ORIENTATION_NW 0xC8

class SpawnLocation
{
public:
    SpawnLocation() :
        _name           ("invalid"),
        _mapID          (0),
        _posX           (0),
        _posY           (0),
        _orientation    (ORIENTATION_SW),
        _region         (RegionCode::ENDGAME),
        _startingLife   (1)
    {}

    SpawnLocation(const std::string& name, uint16_t mapID, uint8_t posX, uint8_t posY, 
                    uint8_t orientation, RegionCode region, uint8_t startingLife) :
        _name           (name),
        _mapID          (mapID),
        _posX           (posX),
        _posY           (posY),
        _orientation    (orientation),
        _region         (region),
        _startingLife   (startingLife)
    {}

    const std::string& getName() const { return _name; }
    uint16_t getMapID() const { return _mapID; }
    uint8_t getPositionX() const { return _posX; }
    uint8_t getPositionY() const { return _posY; }
    uint8_t getOrientation() const { return _orientation; }
    RegionCode getRegion() const { return _region; }
    uint8_t getStartingLife() const { return _startingLife; }

private:
    std::string _name;
    uint16_t _mapID;
    uint8_t _posX;
    uint8_t _posY;
    uint8_t _orientation;
    RegionCode _region;
    uint8_t _startingLife;
};

const std::map<std::string, SpawnLocation>& getAllSpawnLocations();
const SpawnLocation& getSpawnLocationFromName(const std::string& name);
