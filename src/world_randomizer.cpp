#include "world_randomizer.hpp"

#include <algorithm>
#include <sstream>

#include "tools/tools.hpp"

#include "game_text.hpp"
#include "exceptions.hpp"
#include "world_solver.hpp"

WorldRandomizer::WorldRandomizer(World& world, const RandomizerOptions& options) :
    _world          (world),
    _options        (options),
    _rng            (),
    _goldItemsCount (0),
    _debugLogJson   (options.to_json())
{
    this->initFillerItems();
    this->initMandatoryItems();
    this->initInventoryWithStartingItems();
}

void WorldRandomizer::randomize()
{
    uint32_t rngSeed = _options.getSeed();

    // 1st pass: randomizations happening BEFORE randomizing items
    _rng.seed(rngSeed);
    this->randomizeSpawnLocation();
    this->randomizeGoldValues();
    this->randomize_dark_rooms();

    // 2nd pass: randomizing items
    _rng.seed(rngSeed);
    this->randomizeItems();

    // Analyse items required to complete the seed
    _minimalItemsToComplete = _world.getRequiredItemsToComplete();
    _debugLogJson["requiredItems"] = Json::array();
    for (Item* item : _minimalItemsToComplete)
        _debugLogJson["requiredItems"].push_back(item->name());

    // 3rd pass: randomizations happening AFTER randomizing items
    _rng.seed(rngSeed);
    this->randomize_hints();

    if(_options.shuffleTiborTrees())
        this->randomize_tibor_trees();
}

void WorldRandomizer::initFillerItems()
{
    std::map<std::string, uint16_t> fillerItemsDescription;
    if(_options.hasCustomFillerItems())
    {
        fillerItemsDescription = _options.getFillerItems();
    }
    else
    {
        fillerItemsDescription = { 
            {"Life Stock", 80},      {"EkeEke", 55},         {"Golds", 30},          {"Dahl", 16},             
            {"Statue of Gaia", 12},  {"Detox Grass", 11},    {"Golden Statue", 10},  {"Restoration", 10},     
            {"Mind Repair", 7},      {"Anti Paralyze", 7},   {"No Item", 4},         {"Pawn Ticket", 1},
            {"Short Cake", 1},       {"Bell", 1},            {"Blue Ribbon", 1},     {"Death Statue", 1}
        };
    }

    for (auto& [itemName, quantity] : fillerItemsDescription)
    {
        if(itemName == "Golds")
        {
            _goldItemsCount += quantity;
            continue;
        }

        Item* item = _world.item(itemName);
        if(!item)
        {
            std::stringstream msg;
            msg << "Unknown item '" << itemName << "' found in filler items.";
            throw RandomizerException(msg.str());
        }
        
        for(uint16_t i=0 ; i<quantity ; ++i)
            _fillerItems.push_back(item);
    }
}

void WorldRandomizer::initMandatoryItems()
{
    std::map<std::string, uint16_t> mandatoryItemsDescription;
    if(_options.hasCustomMandatoryItems())
    {
        mandatoryItemsDescription = _options.getMandatoryItems();
    }
    else
    {
        mandatoryItemsDescription = {
            {"Magic Sword", 1},      {"Thunder Sword", 1},     {"Sword of Ice", 1},     {"Sword of Gaia", 1},
            {"Steel Breast", 1},     {"Chrome Breast", 1},     {"Shell Breast", 1},     {"Hyper Breast", 1},
            {"Healing Boots", 1},    {"Iron Boots", 1},        {"Fireproof", 1},
            {"Mars Stone", 1},       {"Moon Stone", 1},        {"Saturn Stone", 1},     {"Venus Stone", 1},
            {"Oracle Stone", 1},     {"Statue of Jypta", 1},   {"Spell Book", 1},
        };
    }

    for (auto& [item_name, quantity] : mandatoryItemsDescription)
    {
        Item* item = _world.item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Unknown item '" << item_name << "' found in mandatory items.";
            throw RandomizerException(msg.str());
        }
        
        for(uint16_t i=0 ; i<quantity ; ++i)
            _mandatoryItems.push_back(item);
    }
}

void WorldRandomizer::initInventoryWithStartingItems()
{
    for(const auto& [id, item] : _world.items())
    {
        uint8_t quantity = item->starting_quantity();
        for(uint8_t i=0 ; i<quantity ; ++i)
            _playerInventory.push_back(item);
    }
}


///////////////////////////////////////////////////////////////////////////////////////
///        FIRST PASS RANDOMIZATIONS (before items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeSpawnLocation()
{
    std::vector<std::string> possible_spawn_locations = _options.getPossibleSpawnLocations();
    if(possible_spawn_locations.empty())
    {
        for(auto& [id, spawn] : _world.spawn_locations())
            possible_spawn_locations.push_back(id);
    }

    Tools::shuffle(possible_spawn_locations, _rng);
    SpawnLocation* spawnLoc = _world.spawn_locations().at(possible_spawn_locations[0]);
    _world.active_spawn_location(spawnLoc);
}

void WorldRandomizer::randomizeGoldValues()
{
    constexpr uint16_t averageGoldPerChest = 35;
    constexpr double maxFactorOfTotalGoldValue = 0.16;

    uint16_t totalGoldValue = averageGoldPerChest * _goldItemsCount;

    for (uint8_t i = 0; i < _goldItemsCount ; ++i)
    {
        uint16_t goldValue;

        if (i < _goldItemsCount - 1)
        {
            double proportion = (double) _rng() / (double) _rng.max();
            double factor = (proportion * maxFactorOfTotalGoldValue);

            goldValue = (uint16_t)((double)totalGoldValue * factor);
        }
        else
        {
            goldValue = totalGoldValue;
        }

        if (goldValue == 0)
            goldValue = 1;
        else if (goldValue > 255)
            goldValue = 255;

        totalGoldValue -= goldValue;

        Item* goldItem = _world.add_gold_item(static_cast<uint8_t>(goldValue));
        if(goldItem)
            _fillerItems.push_back(goldItem);
    }
}

void WorldRandomizer::randomize_dark_rooms()
{
    std::vector<WorldRegion*> possible_regions;
    for (auto& [key, region] : _world.regions())
        if (!region->dark_map_ids().empty())
            possible_regions.push_back(region);

    Tools::shuffle(possible_regions, _rng);
    _world.dark_region(possible_regions[0]);

    const std::vector<WorldPath*>& ingoing_paths = _world.dark_region()->ingoing_paths();
    for (WorldPath* path : ingoing_paths)
        path->add_required_item(_world.item(ITEM_LANTERN));
}



///////////////////////////////////////////////////////////////////////////////////////
///        SECOND PASS RANDOMIZATIONS (items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomizeItems()
{
    _regionsToExplore = { _world.active_spawn_location()->region() };
    _exploredRegions.clear();        // Regions already processed by the exploration algorithm
    _itemSourcesToFill.clear();        // Reachable empty item sources which must be filled with a random item
    _playerInventory.clear();        // The current contents of player inventory at the given time in the exploration algorithm
    _pendingPaths.clear();            // Paths leading to potentially unexplored regions, locked behind a key item which is not yet owned by the player

    Tools::shuffle(_fillerItems, _rng);

    this->placeMandatoryItems();

    uint32_t stepCount = 1;
    while (!_pendingPaths.empty() || !_regionsToExplore.empty())
    {
        Json& debugLogStepJson = _debugLogJson["steps"][std::to_string(stepCount)];

        // Explore not yet explored regions, listing all item sources and paths for further exploration and processing
        this->explorationPhase(debugLogStepJson);

        // Try unlocking paths and item sources with newly discovered items in pre-filled item sources (useful for half-plando)
        this->unlockPhase();

        Tools::shuffle(_itemSourcesToFill, _rng);

        // Place one or several key items to unlock access to a path, opening new regions & item sources
        this->placeKeyItemsPhase(debugLogStepJson);

        // Fill a fraction of already available sources with filler items
        if (!_itemSourcesToFill.empty())
        {
            size_t sourcesToFillCount = (size_t)(_itemSourcesToFill.size() * _options.getFillingRate());
            this->placeFillerItemsPhase(debugLogStepJson, sourcesToFillCount);
        }

        // Try unlocking paths and item sources with the newly acquired key item
        this->unlockPhase();
        ++stepCount;
    }

    // Place the remaining filler items, and put Ekeeke in the last empty sources
    Json& debugLogStepJson = _debugLogJson["steps"]["remainder"];
    this->placeFillerItemsPhase(debugLogStepJson, _itemSourcesToFill.size(), _world.item(ITEM_EKEEKE));

    if (!_fillerItems.empty())
    {
        debugLogStepJson["unplacedItems"] = Json::array();
        for (Item* item : _fillerItems)
            debugLogStepJson["unplacedItems"].push_back(item->name());
    }

    _debugLogJson["endState"]["remainingSourcesToFill"] = Json::array();
    for(auto source : _itemSourcesToFill)
        _debugLogJson["endState"]["remainingSourcesToFill"].push_back(source->name());
    _debugLogJson["endState"]["pendingPaths"] = _pendingPaths.size();
}

void WorldRandomizer::placeMandatoryItems()
{
    _debugLogJson["steps"]["0"]["comment"] = "Placing mandatory items";

    // Mandatory items are filler items which are always placed first in the randomization, no matter what
    Tools::shuffle(_mandatoryItems, _rng);

    std::vector<ItemSource*> allEmptyItemSources;
    for (ItemSource* source : _world.item_sources())
        if(!source->item())
            allEmptyItemSources.push_back(source);
    Tools::shuffle(allEmptyItemSources, _rng);

    for (Item* itemToPlace : _mandatoryItems)
    {
        for (ItemSource* source : allEmptyItemSources)
        {
            if (!source->item() && source->is_item_compatible(itemToPlace))
            {
                source->item(itemToPlace);
                _debugLogJson["steps"]["0"]["placedItems"][source->name()] = itemToPlace->name();
                break;
            }
        }
    }
}

void WorldRandomizer::placeFillerItemsPhase(Json& debugLogStepJson, size_t count, Item* lastResortFiller)
{
    for (size_t i=0 ; i<count ; ++i)
    {
        ItemSource* itemSource = _itemSourcesToFill[0];
        
        for (size_t j=0 ; j < _fillerItems.size(); ++j)
        {
            Item* fillerItem = _fillerItems[j];
            if (itemSource->is_item_compatible(fillerItem))
            {
                itemSource->item(fillerItem);
                _fillerItems.erase(_fillerItems.begin() + j);
                _itemSourcesToFill.erase(_itemSourcesToFill.begin());
                break;
            }
        }

        if(itemSource->item() == nullptr)
        {
            // No valid item could be put inside the itemSource...
            if(lastResortFiller)
            {
                // Fill with the "last resort filler" if provided
                itemSource->item(lastResortFiller);
                _itemSourcesToFill.erase(_itemSourcesToFill.begin());
            }
            else
            {
                // No last resort provided, put this item source on the back of the list
                _itemSourcesToFill.erase(_itemSourcesToFill.begin());
                _itemSourcesToFill.push_back(itemSource);
            }
        }

        if(itemSource->item())
            debugLogStepJson["filledSources"][itemSource->name()] = itemSource->item()->name();
    }
}

void WorldRandomizer::explorationPhase(Json& debugLogStepJson)
{
    debugLogStepJson["exploredRegions"] = Json::array();

    while (!_regionsToExplore.empty())
    {
        // Take and erase first region from regions to explore, add it to explored regions set.
        WorldRegion* exploredRegion = *_regionsToExplore.begin();
        _regionsToExplore.erase(exploredRegion);
        _exploredRegions.insert(exploredRegion);
        debugLogStepJson["exploredRegions"].push_back(exploredRegion->name());

        // List item sources to fill from this region.
        const std::vector<ItemSource*> itemSources = exploredRegion->item_sources();
        for (ItemSource* itemSource : itemSources)
        {
            if(itemSource->item())
                _playerInventory.push_back(itemSource->item());    // Non-empty item sources populate player inventory (useful for plandos)
            else
                _itemSourcesToFill.push_back(itemSource);
        }

        // List outgoing paths
        for (WorldPath* outgoingPath : exploredRegion->outgoing_paths())
        {
            // If destination is already pending exploration (through another path) or has already been explored, just ignore it
            WorldRegion* destination = outgoingPath->destination();
            if (_regionsToExplore.contains(destination) || _exploredRegions.contains(destination))
                continue;

            if (outgoingPath->missing_items_to_cross(_playerInventory).empty())
            {
                // For crossable paths, add destination to the list of regions to explore
                _regionsToExplore.insert(destination);
            }
            else
            {
                // For uncrossable blocked paths, add them to a pending list
                _pendingPaths.push_back(outgoingPath);
            }
        }
    }
}

void WorldRandomizer::placeKeyItemsPhase(Json& debugLogStepJson)
{
    if (_pendingPaths.empty())
        return;

    // List all unowned key items, and pick a random one among them
    std::vector<WorldPath*> blockedPaths;
    for (WorldPath* pendingPath : _pendingPaths)
    {
        if(!pendingPath->missing_items_to_cross(_playerInventory).empty())
        {
            // If items are missing to cross this path, add as many entries as the weight of the path to the blockedPaths array
            for(int i=0 ; i<pendingPath->weight() ; ++i)
                blockedPaths.push_back(pendingPath);
        }
    }

    Tools::shuffle(blockedPaths, _rng);
    WorldPath* pathToOpen = blockedPaths[0];
    std::vector<Item*> missingKeyItems = pathToOpen->missing_items_to_cross(_playerInventory);
    for(Item* keyItemToPlace : missingKeyItems)
    {
        // Find a random item source capable of carrying the item
        ItemSource* randomItemSource = nullptr;
        for (uint32_t i = 0; i < _itemSourcesToFill.size(); ++i)
        {
            if (_itemSourcesToFill[i]->is_item_compatible(keyItemToPlace))
            {
                randomItemSource = _itemSourcesToFill[i];
                _itemSourcesToFill.erase(_itemSourcesToFill.begin() + i);
                break;
            }
        }
        if (!randomItemSource)
            throw NoAppropriateItemSourceException();

        // Place the key item in the appropriate source, and also add it to player inventory
        randomItemSource->item(keyItemToPlace);
        _logicalPlaythrough.push_back(randomItemSource);
        _playerInventory.push_back(keyItemToPlace);

        debugLogStepJson["placedKeyItems"][randomItemSource->name()] = keyItemToPlace->name();
    }
}

void WorldRandomizer::unlockPhase()
{
    // Look for unlockable paths...
    for (size_t i=0 ; i < _pendingPaths.size() ; ++i)
    {
        WorldPath* pendingPath = _pendingPaths[i];

        if (pendingPath->missing_items_to_cross(_playerInventory).empty())
        {
            // Path is now unlocked, add destination to regions to explore if it has not yet been explored
            WorldRegion* destination = pendingPath->destination();
            if (!_regionsToExplore.contains(destination) && !_exploredRegions.contains(destination))
                _regionsToExplore.insert(destination);

            // Remove path from pending paths
            _pendingPaths.erase(_pendingPaths.begin()+i);
            --i;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////
///        THIRD PASS RANDOMIZATIONS (after items)
///////////////////////////////////////////////////////////////////////////////////////

void WorldRandomizer::randomize_hints()
{
    this->randomize_lithograph_hint();
    this->randomize_where_is_lithograph_hint();

    Item* hinted_fortune_item = this->randomize_fortune_teller_hint();
    Item* hinted_oracle_stone_item = this->randomize_oracle_stone_hint(hinted_fortune_item);

    this->randomize_sign_hints(hinted_fortune_item, hinted_oracle_stone_item);
}

void WorldRandomizer::randomize_lithograph_hint()
{
    std::stringstream lithograph_hint;
    if(_options.getJewelCount() > MAX_INDIVIDUAL_JEWELS)
    {
        bool first = true;
        std::vector<ItemSource*> allSourcesContainingJewels = _world.item_sources_with_item(_world.item(ITEM_RED_JEWEL));
        for(ItemSource* source : allSourcesContainingJewels)
        {
            if(first)
                first = false;
            else
                lithograph_hint << "\n";
            lithograph_hint << "A jewel is " << this->getRandomHintForItemSource(source) << ".";
        }
    }
    else if(_options.getJewelCount() >= 1)
    {
        lithograph_hint << "Red Jewel is " << this->getRandomHintForItem(_world.item(ITEM_RED_JEWEL)) << ".";
        if(_options.getJewelCount() >= 2)
            lithograph_hint << "\nPurple Jewel is " << this->getRandomHintForItem(_world.item(ITEM_PURPLE_JEWEL)) << ".";
        if(_options.getJewelCount() >= 3)
            lithograph_hint << "\nGreen Jewel is " << this->getRandomHintForItem(_world.item(ITEM_GREEN_JEWEL)) << ".";
        if(_options.getJewelCount() >= 4)
            lithograph_hint << "\nBlue Jewel is " << this->getRandomHintForItem(_world.item(ITEM_BLUE_JEWEL)) << ".";
        if(_options.getJewelCount() >= 5)
            lithograph_hint << "\nYellow Jewel is " << this->getRandomHintForItem(_world.item(ITEM_YELLOW_JEWEL)) << ".";
    }
    else
        lithograph_hint << "This tablet seems of no use...";

    _world.hint_sources().at("Lithograph")->text(lithograph_hint.str());
}

void WorldRandomizer::randomize_where_is_lithograph_hint()
{
    std::stringstream where_is_litho_hint;
    where_is_litho_hint << "The lithograph will help you finding the jewels. It is "
                        << this->getRandomHintForItem(_world.item(ITEM_LITHOGRAPH))
                        << ".";

    _world.hint_sources().at("King Nole's Cave sign")->text(where_is_litho_hint.str());
}

Item* WorldRandomizer::randomize_fortune_teller_hint()
{
    std::vector<uint8_t> hintableItemsByFortuneTeller = { ITEM_GOLA_EYE, ITEM_GOLA_NAIL, ITEM_GOLA_FANG, ITEM_GOLA_HORN };
    Tools::shuffle(hintableItemsByFortuneTeller, _rng);
    
    Item* hinted_item = _world.item(*(hintableItemsByFortuneTeller.begin()));

    std::string fortuneItemName;
    if (hinted_item == _world.item(ITEM_GOLA_EYE))
        fortuneItemName = "an eye";
    else if (hinted_item == _world.item(ITEM_GOLA_NAIL))
        fortuneItemName = "a nail";
    else if (hinted_item == _world.item(ITEM_GOLA_FANG))
        fortuneItemName = "a fang";
    else if (hinted_item == _world.item(ITEM_GOLA_HORN))
        fortuneItemName = "a horn";

    std::stringstream fortune_teller_hint;
    fortune_teller_hint << "I see " << fortuneItemName << " " << this->getRandomHintForItem(hinted_item) << ".";
    _world.hint_sources().at("Mercator fortune teller")->text(fortune_teller_hint.str());

    return hinted_item;
}

Item* WorldRandomizer::randomize_oracle_stone_hint(Item* forbiddenFortuneTellerItem)
{
    UnsortedSet<Item*> forbiddenOracleStoneItems = {
        forbiddenFortuneTellerItem, _world.item(ITEM_RED_JEWEL), _world.item(ITEM_PURPLE_JEWEL),
        _world.item(ITEM_GREEN_JEWEL), _world.item(ITEM_BLUE_JEWEL), _world.item(ITEM_YELLOW_JEWEL)
    };

    // Also excluding items strictly needed to get to Oracle Stone's location
    std::vector<ItemSource*> sources = _world.item_sources_with_item(_world.item(ITEM_ORACLE_STONE));
    WorldRegion* first_item_region = sources.at(0)->region();
    if(first_item_region)
    {
        std::vector<Item*> min_items_to_reach = _world.findSmallestInventoryToReachRegion(first_item_region);
        for (Item* item : min_items_to_reach)
            forbiddenOracleStoneItems.insert(item);
    }

    std::vector<Item*> hintable_items;
    for (Item* item : _minimalItemsToComplete)
    {
        if(!forbiddenOracleStoneItems.contains(item))
            hintable_items.push_back(item);
    }
    
    if (!hintable_items.empty())
    {
        Tools::shuffle(hintable_items, _rng);
        Item* hinted_item = hintable_items[0];

        std::stringstream oracle_stone_hint;
        oracle_stone_hint << "You will need " << hinted_item->name() << ". It is " << this->getRandomHintForItem(hinted_item) << ".";
        _world.hint_sources().at("Oracle Stone")->text(oracle_stone_hint.str());

        return hinted_item;
    }
    
    _world.hint_sources().at("Oracle Stone")->text("The stone looks blurry. It looks like it won't be of any use...");
    return nullptr;
}


void WorldRandomizer::randomize_sign_hints(Item* hintedFortuneItem, Item* hintedOracleStoneItem)
{
    // A shuffled list of macro regions, useful for the "barren / useful region" hints
    std::vector<WorldMacroRegion*> macroRegionsAvailableForHints;
    for (WorldMacroRegion* macroRegion : _world.macro_regions())
        macroRegionsAvailableForHints.push_back(macroRegion);
    Tools::shuffle(macroRegionsAvailableForHints, _rng);

    // A shuffled list of potentially optional items, useful for the "this item will be useful / useless" hints
    std::vector<uint8_t> hintableItemsNecessity = {
        ITEM_BUYER_CARD, ITEM_EINSTEIN_WHISTLE, ITEM_ARMLET, ITEM_GARLIC, ITEM_IDOL_STONE, ITEM_CASINO_TICKET, ITEM_LOGS
    };
    Tools::shuffle(hintableItemsNecessity, _rng);

    // A shuffled list of items which location is interesting, useful for the "item X is in Y" hints
    std::vector<uint8_t> hintableItemLocations = {
        ITEM_SPIKE_BOOTS,        ITEM_AXE_MAGIC,        ITEM_BUYER_CARD,    ITEM_GARLIC,
        ITEM_EINSTEIN_WHISTLE,    ITEM_ARMLET,        ITEM_IDOL_STONE,
        ITEM_THUNDER_SWORD,        ITEM_HEALING_BOOTS,    ITEM_VENUS_STONE,    ITEM_STATUE_JYPTA,
        ITEM_SUN_STONE,            ITEM_KEY,            ITEM_SAFETY_PASS,    ITEM_LOGS,
        ITEM_GOLA_EYE,            ITEM_GOLA_NAIL,        ITEM_GOLA_FANG,        ITEM_GOLA_HORN
    };
    
    auto it = std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedFortuneItem->id());
    if(it != hintableItemLocations.end())
        hintableItemLocations.erase(it);

    it = std::find(hintableItemLocations.begin(), hintableItemLocations.end(), hintedOracleStoneItem->id());
    if(it != hintableItemLocations.end())
        hintableItemLocations.erase(it);

    Tools::shuffle(hintableItemLocations, _rng);

    for (auto& [k, hint_source] : _world.hint_sources())
    {
        if(hint_source->special())
            continue;

        std::string hintText;
        double randomNumber = (double) _rng() / (double) _rng.max();
        WorldRegion* region = hint_source->region();
        std::vector<Item*> itemsAlreadyObtainedAtSign = _world.findSmallestInventoryToReachRegion(region);
        int nextEligibleHintableItemsNecessityPos = this->getNextEligibleHintableItemPos(hintableItemsNecessity, itemsAlreadyObtainedAtSign);

        // "Barren / pleasant surprise" (30%)
        if (randomNumber < 0.3 && !macroRegionsAvailableForHints.empty())
        {
            WorldMacroRegion* macroRegion = *macroRegionsAvailableForHints.begin();
            if (_world.is_macro_region_avoidable(macroRegion))
                hintText = "What you are looking for is not in " + macroRegion->name() + ".";
            else
                hintText = "You might have a pleasant surprise wandering in " + macroRegion->name() + ".";

            macroRegionsAvailableForHints.erase(macroRegionsAvailableForHints.begin());
        }
        // "You will / won't need {item} to finish" (25%)
        else if (randomNumber < 0.55 && nextEligibleHintableItemsNecessityPos >=0)
        {
            Item* hintedItem = _world.item(hintableItemsNecessity.at(nextEligibleHintableItemsNecessityPos));
            if (!_world.is_item_avoidable(hintedItem))
                hintText = "You will need " + hintedItem->name() + " in your quest to King Nole's treasure.";
            else
                hintText = hintedItem->name() + " is not required in your quest to King Nole's treasure.";

            hintableItemsNecessity.erase(hintableItemsNecessity.begin() + nextEligibleHintableItemsNecessityPos);
        }
        // "You shall find {item} in {place}" (45%)
        else if (!hintableItemLocations.empty())
        {
            Item* hintedItem = nullptr;
            for (uint32_t i = 0; i < hintableItemLocations.size(); ++i)
            {
                Item* testedItem = _world.item(hintableItemLocations[i]);
                if (std::find(itemsAlreadyObtainedAtSign.begin(), itemsAlreadyObtainedAtSign.end(), testedItem) == itemsAlreadyObtainedAtSign.end())
                {
                    // If item was not already obtained at sign, we can hint it
                    hintedItem = testedItem;
                    hintableItemLocations.erase(hintableItemLocations.begin() + i);
                    break;
                }
            }

            if (hintedItem) {
                hintText = "You shall find " + hintedItem->name() + " " + this->getRandomHintForItem(hintedItem) + ".";
            } else {
                hintText = "This sign has been damaged in a way that makes it unreadable.";
            }
        }

        hint_source->text(hintText);
    }
}

uint32_t WorldRandomizer::getNextEligibleHintableItemPos(std::vector<uint8_t> hintableItemsNecessity, const std::vector<Item*>& itemsAlreadyObtainedAtSign)
{
    for (uint32_t i = 0; i < hintableItemsNecessity.size(); ++i)
    {
        Item* testedItem = _world.item(hintableItemsNecessity[i]);
        if (std::find(itemsAlreadyObtainedAtSign.begin(), itemsAlreadyObtainedAtSign.end(), testedItem) == itemsAlreadyObtainedAtSign.end())
            return i;
    }
    return -1;
}

std::string WorldRandomizer::getRandomHintForItem(Item* item)
{
    std::vector<ItemSource*> sources = _world.item_sources_with_item(item);
    if(sources.empty())
        return "in an unknown place";

    Tools::shuffle(sources, _rng);
    ItemSource* randomSource = *sources.begin();
    return this->getRandomHintForItemSource(randomSource);
}

std::string WorldRandomizer::getRandomHintForItemSource(ItemSource* itemSource)
{
    const std::vector<std::string>& regionHints = itemSource->region()->hints();
    const std::vector<std::string>& sourceHints = itemSource->hints();
        
    std::vector<std::string> allHints;
    allHints.insert(allHints.end(), regionHints.begin(), regionHints.end());
    allHints.insert(allHints.end(), sourceHints.begin(), sourceHints.end());
    
    if(allHints.empty())
        return "in an unknown place";

    Tools::shuffle(allHints, _rng);
    return *allHints.begin();
}


void WorldRandomizer::randomize_tibor_trees()
{
    const std::vector<WorldTeleportTree*>& trees = _world.teleport_trees();

    std::vector<uint16_t> teleport_tree_map_ids;
    for (WorldTeleportTree* tree : trees)
        teleport_tree_map_ids.push_back(tree->tree_map_id());
    
    Tools::shuffle(teleport_tree_map_ids, _rng);
    for (uint8_t i = 0; i < trees.size(); ++i)
        trees.at(i)->tree_map_id(teleport_tree_map_ids[i]);
}

Json WorldRandomizer::getPlaythroughAsJson() const
{
    Json json;

    // Filter the logical playthrough to keep only strictly needed key items
    for(ItemSource* source : _logicalPlaythrough)
    {
        Item* keyItemInSource = source->item();
        if(std::find(_minimalItemsToComplete.begin(), _minimalItemsToComplete.end(), keyItemInSource) != _minimalItemsToComplete.end())
            json[source->name()] = keyItemInSource->name();
    }

    return json;
}
