#pragma once

#include "Enums/ItemSourceCodes.hpp"
#include "Enums/RegionCodes.hpp"
#include "ItemSources.hpp"
#include "Item.hpp"
#include "GameText.hpp"
#include "WorldRegion.hpp"
#include "WorldPath.hpp"
#include "WorldMacroRegion.hpp"
#include "RandomizerOptions.hpp"
#include "TreeMap.hpp"
#include "HintSign.hpp"
#include "Extlibs/json.hpp"

constexpr uint8_t MAX_INDIVIDUAL_JEWELS = 5;

class World
{
public:
    World(const RandomizerOptions& options);
    ~World();

    void addItem(Item* item) { items[item->getID()] = item; }
    Item* addGoldItem(uint8_t worth);

    Item* getItemByName(const std::string& name) const;
    WorldRegion* getRegionByName(const std::string& name) const;

    WorldRegion* getRegionForItem(Item* item);
    std::vector<ItemSource*> getItemSourcesContainingItem(Item* item);

    void setSpawnLocation(const SpawnLocation& spawnLocation) { _spawnLocation = spawnLocation; }
    const SpawnLocation& getSpawnLocation() const { return _spawnLocation; }
    WorldRegion* getSpawnRegion() const { return regions.at(_spawnLocation.getRegion()); }

    void writeToROM(md::ROM& rom);

    Json toJSON() const;
    void parseJSON(const Json& json);
    Item* parseItemFromName(const std::string& itemName);

    std::vector<Item*> findSmallestInventoryToReachRegion(WorldRegion* endRegion) const;
    std::vector<Item*> getRequiredItemsToComplete() const { return this->findSmallestInventoryToReachRegion(regions.at(RegionCode::ENDGAME)); }
  
    bool isMacroRegionAvoidable(WorldMacroRegion* macroRegion) const;
    bool isItemAvoidable(Item* item) const;

private:
    void initItems(const RandomizerOptions& options);
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
    std::vector<TreeMap> treeMaps;

    WorldRegion* darkenedRegion;

    std::vector<std::string> jewelHints;
    std::string whereIsLithographHint;
    std::string fortuneTellerHint;
    std::string oracleStoneHint;

private:
    const RandomizerOptions& _options;

    SpawnLocation _spawnLocation;
};
