#pragma once

#include "extlibs/json.hpp"

#include "model/item.hpp"
#include "model/item_source.hpp"
#include "model/world_region.hpp"
#include "model/world_path.hpp"
#include "model/spawn_location.hpp"
#include "model/hint_source.hpp"
#include "model/world_macro_region.hpp"
#include "model/world_teleport_tree.hpp"

#include "randomizer_options.hpp"

constexpr uint8_t MAX_INDIVIDUAL_JEWELS = 5;

class World
{
private:
    std::map<uint8_t, Item*> _items;
    std::map<std::string, WorldRegion*> _regions;
    std::map<std::pair<WorldRegion*, WorldRegion*>, WorldPath*> _paths;
    std::vector<ItemSource*> _item_sources;
    std::map<std::string, SpawnLocation*> _spawn_locations;
    std::map<std::string, HintSource*> _hint_sources;
    std::vector<WorldMacroRegion*> _macro_regions;
    std::vector<WorldTeleportTree*> _teleport_trees;
    std::vector<std::string> _game_strings;
    std::vector<uint8_t> _fahl_enemies;

    SpawnLocation* _active_spawn_location;
    WorldRegion* _dark_region;

    const RandomizerOptions& _options;

public:
    World(const md::ROM& rom, const RandomizerOptions& options);
    ~World();

    const std::map<uint8_t, Item*> items() const { return _items; }
    Item* item(uint8_t id) const { return _items.at(id); }
    Item* item(const std::string& name) const;
    Item* add_item(Item* item) { _items[item->id()] = item; return item; }
    Item* add_gold_item(uint8_t worth);

    const std::map<std::string, WorldRegion*>& regions() const { return _regions; }
    WorldRegion* region(const std::string& id) const { return _regions.at(id); }
    WorldRegion* first_region_with_item(Item* item);

    const std::map<std::pair<WorldRegion*, WorldRegion*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldRegion* origin, WorldRegion* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    std::vector<ItemSource*> item_sources_with_item(Item* item);

    const std::map<std::string, SpawnLocation*>& spawn_locations() const { return _spawn_locations; }
    void add_spawn_location(SpawnLocation* spawn) { _spawn_locations[spawn->id()] = spawn; }
    
    const std::map<std::string, HintSource*>& hint_sources() const { return _hint_sources; }
    const std::vector<WorldMacroRegion*> macro_regions() const { return _macro_regions; }
    const std::vector<WorldTeleportTree*>& teleport_trees() const { return _teleport_trees; }
    const std::vector<std::string>& game_strings() const { return _game_strings; }

    void active_spawn_location(SpawnLocation* spawn) { _active_spawn_location = spawn; }
    SpawnLocation* active_spawn_location() const { return _active_spawn_location; }

    WorldRegion* dark_region() { return _dark_region; }
    void dark_region(WorldRegion* region) { _dark_region = region; }

    const std::vector<uint8_t>& fahl_enemies() const { return _fahl_enemies; }
    void add_fahl_enemy(uint8_t enemy_id) { if(_fahl_enemies.size() < 0x50) _fahl_enemies.push_back(enemy_id); }

    void write_to_rom(md::ROM& rom);

    Json to_json() const;
    void parse_json(const Json& json);
    Item* parse_item_from_name(const std::string& itemName);

    bool is_macro_region_avoidable(WorldMacroRegion* macro_region) const;
    bool is_item_avoidable(Item* item) const;

    void output_model();
    void output_graphviz();

private:
    void init_items();
    void init_item_sources();
    void init_regions();
    void init_paths();
    void init_macro_regions();
    void init_spawn_locations();
    void init_tree_maps();
    void init_game_strings(const md::ROM& rom);
    void init_hint_sources();
};
