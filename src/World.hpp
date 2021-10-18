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
#include "Extlibs/json.hpp"

constexpr uint8_t GOLD_SOURCES_COUNT = 30;
constexpr uint8_t MAX_INDIVIDUAL_JEWELS = 3;

class World
{
public:
    World(const RandomizerOptions& options);
    ~World();

    void addItem(Item* item) { items[item->getID()] = item; }
  
    Item* getItemByName(const std::string& name) const;
    WorldRegion* getRegionByName(const std::string& name) const;

    WorldRegion* getRegionForItem(Item* item);
    std::vector<ItemSource*> getItemSourcesContainingItem(Item* item);

    const std::vector<Item*>& getFillerItemsList() const { return _fillerItems; }
    const std::vector<Item*>& getPriorityItemsList() const { return _priorityItems; }

    void writeToROM(md::ROM& rom);

    Json toJSON() const;
    void parseJSON(const Json& json);

private:
    void initItems(const RandomizerOptions& options);
    void initFillerItemsList();
    void initPriorityItemsList();

    void initChests();
    void initGroundItems();
    void initShops();
    void initNPCRewards();

    void initRegions();
    void initRegionPaths(const RandomizerOptions& options);

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
    
    std::vector<std::string> jewelHints;
    std::string whereIsLithographHint;
    std::string fortuneTellerHint;
    std::string oracleStoneHint;

    std::vector<TreeMap> treeMaps;

    SpawnLocation spawnLocation;

private:
    const RandomizerOptions& _options;

    std::vector<Item*> _fillerItems;
    std::vector<Item*> _priorityItems;
};
