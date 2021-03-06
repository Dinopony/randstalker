#pragma once

#include "Enums/ItemSourceCodes.hpp"
#include "Enums/RegionCodes.hpp"
#include "ItemSources.hpp"
#include "Item.hpp"
#include "GameText.hpp"
#include "WorldRegion.hpp"
#include "WorldMacroRegion.hpp"
#include "RandomizerOptions.hpp"
#include "TreeMap.hpp"
#include "HintSign.hpp"

constexpr auto GOLD_SOURCES_COUNT = 30;

struct World
{
public:
    World(const RandomizerOptions& options);
    ~World();

    void addItem(Item* item) { items[item->getID()] = item; }

    WorldRegion* getRegionForItem(Item* item);
    ItemSource* getItemSourceForItem(Item* item);

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
    std::vector<WorldMacroRegion*> macroRegions;
    std::vector<ItemShop*> shops;

    std::vector<HintSign*> hintSigns;

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
