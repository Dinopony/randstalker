#pragma once

#include "Constants/ItemSourceCodes.h"
#include "Constants/RegionCodes.h"
#include "ItemSources/AbstractItemSource.h"

#include "Item.h"
#include "GameText.h"
#include "WorldRegion.h"
#include "RandomizerOptions.h"
#include "TreeMap.h"
#include "ItemShop.h"

constexpr auto GOLD_SOURCES_COUNT = 30;

struct World
{
public:
    World(const RandomizerOptions& options);
    ~World();

    void writeToROM(md::ROM& rom);

private:
    void initItems();
    void replaceArmorsByArmorUpgrades();

    void initChests();
    void initGroundItems();
    void initShops();
    void initNPCRewards();

    void initRegions();
    void initRegionPaths();

    void initHints();

//    void initDarkRooms();

    void initTreeMaps();

public:
	std::map<uint8_t, Item*> items;
	std::map<ItemSourceCode, AbstractItemSource*> itemSources;
    std::map<RegionCode, WorldRegion*> regions;
    std::vector<ItemShop*> shops;

    WorldRegion* darkenedRegion;
    GameText lithographHint;

    std::vector<TreeMap> treeMaps;

    uint16_t spawnMapID;
    uint8_t spawnX, spawnZ;
};
