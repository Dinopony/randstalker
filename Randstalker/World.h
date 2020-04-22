#pragma once

#include "ItemSourceCodes.h"
#include "RegionCodes.h"
#include "ItemSources.h"
#include "Item.h"
#include "GameText.h"
#include "WorldRegion.h"
#include "RandomizerOptions.h"
#include "TreeMap.h"

constexpr auto GOLD_SOURCES_COUNT = 30;

struct World
{
public:
    World(const RandomizerOptions& options);
    ~World();

    void addItem(Item* item) { items[item->getID()] = item; }

    void writeToROM(md::ROM& rom);

private:
    void initItems(bool useArmorUpgrades);

    void initChests();
    void initGroundItems();
    void initShops();
    void initNPCRewards();

    void initRegions();
    void initRegionPaths();

    void initRegionHints();
    void initHintSigns(bool replaceOGHints);

    void initDarkRooms();

    void initTreeMaps();

public:
	std::map<uint8_t, Item*> items;
	std::map<ItemSourceCode, ItemSource*> itemSources;
    std::map<RegionCode, WorldRegion*> regions;
    std::map<std::string, std::vector<WorldRegion*>> macroRegions;
    std::vector<ItemShop*> shops;

    std::map<uint32_t, std::string> hintSigns;

    WorldRegion* darkenedRegion;
    
    std::string redJewelHint;
    std::string purpleJewelHint;
    std::string whereIsLithographHint;
    std::string fortuneTellerHint;
    std::string oracleStoneHint;

    std::vector<std::string> textLines;

    std::vector<TreeMap> treeMaps;

    SpawnLocation spawnLocation;
};
