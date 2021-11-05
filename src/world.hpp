#pragma once

#include "extlibs/json.hpp"
#include "tools/megadrive/rom.hpp"
#include <map>
#include <vector>

constexpr uint8_t MAX_INDIVIDUAL_JEWELS = 5;

class RandomizerOptions;
class WorldNode;
class WorldPath;
class WorldRegion;
class SpawnLocation;
class HintSource;
class ItemSource;
class Item;
class WorldTeleportTree;
class Entity;
class Map;

class World
{
private:
    std::map<uint8_t, Item*> _items;
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<ItemSource*> _item_sources;
    std::map<std::string, SpawnLocation*> _spawn_locations;
    std::map<std::string, HintSource*> _hint_sources;
    std::vector<WorldRegion*> _regions;
    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> _teleport_tree_pairs;
    std::vector<std::string> _game_strings;
    std::map<uint8_t, Entity*> _entities;
    std::map<uint16_t, Map*> _maps;
    std::vector<Entity*> _fahl_enemies;

    SpawnLocation* _active_spawn_location;
    WorldRegion* _dark_region;

    const RandomizerOptions& _options;

public:
    World(const md::ROM& rom, const RandomizerOptions& options);
    ~World();

    const std::map<uint8_t, Item*> items() const { return _items; }
    Item* item(uint8_t id) const { return _items.at(id); }
    Item* item(const std::string& name) const;
    Item* add_item(Item* item);
    Item* add_gold_item(uint8_t worth);
    std::vector<Item*> starting_inventory() const;

    const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    WorldNode* node(const std::string& id) const { return _nodes.at(id); }
    WorldNode* first_node_with_item(Item* item);

    const std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    std::vector<ItemSource*> item_sources_with_item(Item* item);

    const std::map<std::string, SpawnLocation*>& spawn_locations() const { return _spawn_locations; }
    void add_spawn_location(SpawnLocation* spawn);
    
    const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& teleport_tree_pairs() const { return _teleport_tree_pairs; }
    void teleport_tree_pairs(const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& new_pairs) { _teleport_tree_pairs = new_pairs; }
    void add_tree_logic_paths();

    const std::vector<WorldRegion*> regions() const { return _regions; }
    WorldRegion* region(const std::string& name) const;

    const std::map<std::string, HintSource*>& hint_sources() const { return _hint_sources; }
    const std::vector<std::string>& game_strings() const { return _game_strings; }

    void active_spawn_location(SpawnLocation* spawn) { _active_spawn_location = spawn; }
    SpawnLocation* active_spawn_location() const { return _active_spawn_location; }
    WorldNode* spawn_node() const;
    WorldNode* end_node() const;

    WorldRegion* dark_region() const { return _dark_region; }
    void dark_region(WorldRegion* region) { _dark_region = region; }

    const std::map<uint8_t, Entity*>& entities() const { return _entities; }
    Entity* entity(uint8_t id) const { return _entities.at(id); }
    Entity* entity(const std::string& name) const;

    const std::vector<Entity*>& fahl_enemies() const { return _fahl_enemies; }
    void add_fahl_enemy(Entity* enemy) { _fahl_enemies.push_back(enemy); }

    void write_to_rom(md::ROM& rom);

    Json to_json() const;
    void parse_json(const Json& json);
    Item* parse_item_from_name(const std::string& itemName);

    bool is_region_avoidable(WorldRegion* region) const;
    bool is_item_avoidable(Item* item) const;

    void output_model();
    void output_graphviz();
    void output_maps_to_graphviz();
private:
    void init_items();
    void init_item_sources();
    void init_nodes();
    void init_paths();
    void init_regions();
    void init_spawn_locations();
    void init_hint_sources();
    void init_teleport_trees();
    void init_game_strings(const md::ROM& rom);
    void init_entities(const md::ROM& rom);
    void init_maps(const md::ROM& rom);

    void write_items(md::ROM& rom);
    void write_maps(md::ROM& rom);
};
