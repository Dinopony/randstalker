#pragma once

#include <json.hpp>
#include <md_tools.hpp>
#include <map>
#include <vector>

#include "spawn_location.hpp"
#include "map_connection.hpp"
#include "../tools/flag.h"

class WorldNode;
class WorldPath;
class WorldRegion;
class SpawnLocation;
class HintSource;
class ItemSource;
class Item;
class WorldTeleportTree;
class EntityType;
class Map;
class MapPalette;

class World
{
private:
    std::map<uint8_t, Item*> _items;
    std::vector<ItemSource*> _item_sources;
    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> _teleport_tree_pairs;
    std::vector<std::string> _game_strings;
    std::map<uint8_t, EntityType*> _entity_types;
    std::map<uint16_t, Map*> _maps;
    std::vector<MapConnection> _map_connections;
    std::vector<MapPalette*> _map_palettes;
    std::vector<EntityType*> _fahl_enemies;
    std::vector<uint16_t> _dark_maps;
    SpawnLocation _spawn_location;
    std::vector<Flag> _starting_flags;
    uint16_t _starting_golds = 0;
    uint8_t _custom_starting_life = 0;

public:
    World(const md::ROM& rom);
    ~World();

    void write_to_rom(md::ROM& rom);

    const std::map<uint8_t, Item*>& items() const { return _items; }
    std::map<uint8_t, Item*>& items() { return _items; }
    Item* item(uint8_t id) const { return _items.at(id); }
    Item* item(const std::string& name) const;
    Item* add_item(Item* item);
    Item* add_gold_item(uint8_t worth);
    std::vector<Item*> starting_inventory() const;

    const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    std::vector<ItemSource*>& item_sources() { return _item_sources; }
    std::vector<ItemSource*> item_sources_with_item(Item* item);
    
    const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& teleport_tree_pairs() const { return _teleport_tree_pairs; }
    void teleport_tree_pairs(const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& new_pairs) { _teleport_tree_pairs = new_pairs; }

    const std::vector<std::string>& game_strings() const { return _game_strings; }
    std::vector<std::string>& game_strings() { return _game_strings; }

    const std::vector<uint16_t>& dark_maps() const { return _dark_maps; }
    void dark_maps(const std::vector<uint16_t>& dark_maps) { _dark_maps = dark_maps; }

    void spawn_location(const SpawnLocation& spawn) { _spawn_location = spawn; }
    const SpawnLocation& spawn_location() const { return _spawn_location; }

    const std::vector<Flag>& starting_flags() const { return _starting_flags; }
    std::vector<Flag>& starting_flags() { return _starting_flags; }

    uint16_t starting_golds() const { return _starting_golds; }
    void starting_golds(uint16_t golds) { _starting_golds = golds; }

    uint8_t starting_life() const;
    uint8_t custom_starting_life() const { return _custom_starting_life; }
    void custom_starting_life(uint8_t life) { _custom_starting_life = life; }

    const std::map<uint8_t, EntityType*>& entity_types() const { return _entity_types; }
    EntityType* entity_type(uint8_t id) const { return _entity_types.at(id); }
    EntityType* entity_type(const std::string& name) const;

    const std::map<uint16_t, Map*>& maps() const { return _maps; }
    Map* map(uint16_t map_id) const { return _maps.at(map_id); }
    void set_map(uint16_t map_id, Map* map);

    const std::vector<MapConnection>& map_connections() const { return _map_connections; }
    std::vector<MapConnection>& map_connections() { return _map_connections; }
    MapConnection& map_connection(uint16_t map_id_1, uint16_t map_id_2);
    void swap_map_connections(uint16_t map_id_1, uint16_t map_id_2, uint16_t map_id_3, uint16_t map_id_4);
    
    const std::vector<MapPalette*>& map_palettes() const { return _map_palettes; }
    std::vector<MapPalette*>& map_palettes() { return _map_palettes; }
    MapPalette* map_palette(uint8_t id) { return _map_palettes.at(id); }
    uint8_t map_palette_id(MapPalette* palette) const;

    const std::vector<EntityType*>& fahl_enemies() const { return _fahl_enemies; }
    void add_fahl_enemy(EntityType* enemy) { _fahl_enemies.emplace_back(enemy); }

    // void parse_json(const Json& json);
    Item* parse_item_from_name(const std::string& itemName);

private:
    void init_items();
    void init_item_sources();
    void init_teleport_trees();
    void init_game_strings(const md::ROM& rom);
    void init_entity_types(const md::ROM& rom);

    void clean_unused_palettes();
};
