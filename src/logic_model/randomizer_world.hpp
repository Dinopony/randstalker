#pragma once

#include <map>
#include <vector>
#include <string>
#include <landstalker-lib/constants/item_codes.hpp>
#include <landstalker-lib/model/world.hpp>
#include "spawn_location.hpp"

class WorldNode;
class WorldPath;
class WorldRegion;
class HintSource;
class Entity;

class RandomizerWorld : public World {
private:
    std::vector<ItemSource*> _item_sources;

    std::map<std::string, WorldNode*> _nodes;
    std::vector<WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;

    std::map<std::string, SpawnLocation*> _available_spawn_locations;
    const SpawnLocation* _spawn_location = nullptr;

    std::array<uint16_t, ITEM_COUNT> _item_quantities;

    std::vector<HintSource*> _hint_sources;
    std::vector<HintSource*> _used_hint_sources;

    WorldRegion* _dark_region = nullptr;

    std::vector<EntityType*> _fahl_enemies;

    std::map<Entity*, std::string> _custom_dialogues;

    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> _teleport_tree_pairs;
    std::vector<Item*> _archipelago_items;

public:
    RandomizerWorld();
    ~RandomizerWorld();

    [[nodiscard]] std::map<std::string, uint8_t> item_names(bool strict = false) const;

    [[nodiscard]] const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    [[nodiscard]] std::vector<ItemSource*>& item_sources() { return _item_sources; }
    [[nodiscard]] ItemSource* item_source(const std::string& name) const;
    [[nodiscard]] std::vector<ItemSource*> item_sources_with_item(Item* item);

    [[nodiscard]] const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    [[nodiscard]] WorldNode* node(const std::string& id) const { return _nodes.at(id); }

    [[nodiscard]] const std::vector<WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    [[nodiscard]] const std::vector<WorldRegion*>& regions() const { return _regions; }
    [[nodiscard]] WorldRegion* region(const std::string& name) const;

    [[nodiscard]] const std::map<std::string, SpawnLocation*>& available_spawn_locations() const { return _available_spawn_locations; }
    [[nodiscard]] std::vector<std::string> spawn_location_names() const;
    void add_spawn_location(SpawnLocation* spawn);

    [[nodiscard]] const SpawnLocation* spawn_location() const { return _spawn_location; }
    void spawn_location(const SpawnLocation* spawn);

    [[nodiscard]] const std::array<uint16_t, ITEM_COUNT>& item_quantities() const { return _item_quantities; }
    [[nodiscard]] uint16_t item_quantity(uint8_t item_id) const { return _item_quantities.at(item_id); }
    void item_quantity(uint8_t item_id, uint16_t quantity) { _item_quantities[item_id] = quantity; }

    [[nodiscard]] const std::vector<HintSource*>& hint_sources() const { return _hint_sources; }
    [[nodiscard]] HintSource* hint_source(const std::string& name) const;
    void add_hint_source(HintSource* hint_source) { _hint_sources.emplace_back(hint_source); }
    [[nodiscard]] const std::vector<HintSource*>& used_hint_sources() const { return _used_hint_sources; }
    void add_used_hint_source(HintSource* hint_source) { _used_hint_sources.emplace_back(hint_source); }

    [[nodiscard]] WorldNode* spawn_node() const { return _nodes.at(_spawn_location->node_id()); }
    [[nodiscard]] WorldNode* end_node() const { return _nodes.at("end"); }

    [[nodiscard]] WorldRegion* dark_region() const { return _dark_region; }
    void dark_region(WorldRegion* region);

    [[nodiscard]] const std::vector<EntityType*>& fahl_enemies() const { return _fahl_enemies; }
    void add_fahl_enemy(EntityType* enemy) { _fahl_enemies.emplace_back(enemy); }

    [[nodiscard]] const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& teleport_tree_pairs() const { return _teleport_tree_pairs; }
    void teleport_tree_pairs(const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& new_pairs) { _teleport_tree_pairs = new_pairs; }

    const std::map<Entity*, std::string>& custom_dialogues() { return _custom_dialogues; }
    void add_custom_dialogue(Entity* entity, const std::string& text);
    void add_custom_dialogue_raw(Entity* entity, const std::string& text);

    void add_paths_for_tree_connections(bool require_tibor_access);

    Item* add_archipelago_item(std::string item_name, std::string player_name, bool use_shop_naming);

    void load_model_from_json(bool lite_mode = false);

private:
    void load_additional_item_data();
    void load_item_sources(bool lite_mode);
    void load_nodes();
    void load_paths();
    void load_regions();
    void load_spawn_locations();
    void load_hint_sources();
    void load_teleport_trees();
};
