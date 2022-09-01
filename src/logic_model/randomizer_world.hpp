#pragma once

#include <map>
#include <vector>
#include <string>
#include <landstalker_lib/model/world.hpp>
#include "item_distribution.hpp"

class WorldNode;
class WorldPath;
class WorldRegion;
class HintSource;

class RandomizerWorld : public World {
private:
    std::vector<ItemSource*> _item_sources;

    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;
    std::map<std::string, SpawnLocation*> _available_spawn_locations;
    std::array<ItemDistribution, ITEM_COUNT+1> _item_distributions;

    std::vector<HintSource*> _hint_sources;
    std::vector<HintSource*> _used_hint_sources;

    WorldNode* _spawn_node = nullptr;
    WorldRegion* _dark_region = nullptr;

    std::vector<EntityType*> _fahl_enemies;

    std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>> _teleport_tree_pairs;

public:
    RandomizerWorld() = default;
    ~RandomizerWorld();

    [[nodiscard]] std::array<std::string, ITEM_COUNT+1> item_names() const;

    [[nodiscard]] const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    [[nodiscard]] std::vector<ItemSource*>& item_sources() { return _item_sources; }
    [[nodiscard]] ItemSource* item_source(const std::string& name) const;
    [[nodiscard]] std::vector<ItemSource*> item_sources_with_item(Item* item);

    [[nodiscard]] const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    [[nodiscard]] WorldNode* node(const std::string& id) const { return _nodes.at(id); }

    [[nodiscard]] const std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    [[nodiscard]] const std::vector<WorldRegion*>& regions() const { return _regions; }
    [[nodiscard]] WorldRegion* region(const std::string& name) const;

    [[nodiscard]] const std::map<std::string, SpawnLocation*>& available_spawn_locations() const { return _available_spawn_locations; }
    void add_spawn_location(SpawnLocation* spawn);

    [[nodiscard]] const SpawnLocation& spawn_location() const override { return World::spawn_location(); }
    void spawn_location(const SpawnLocation& spawn) override;

    [[nodiscard]] const std::array<ItemDistribution, ITEM_COUNT+1>& item_distributions() const { return _item_distributions; }
    [[nodiscard]] const ItemDistribution* item_distribution(uint8_t item_id) const { return &_item_distributions[item_id]; }
    [[nodiscard]] ItemDistribution* item_distribution(uint8_t item_id) { return &_item_distributions[item_id]; }
    [[nodiscard]] std::map<uint8_t, uint16_t> item_quantities() const;

    [[nodiscard]] const std::vector<HintSource*>& hint_sources() const { return _hint_sources; }
    [[nodiscard]] HintSource* hint_source(const std::string& name) const;
    void add_hint_source(HintSource* hint_source) { _hint_sources.emplace_back(hint_source); }
    [[nodiscard]] const std::vector<HintSource*>& used_hint_sources() const { return _used_hint_sources; }
    void add_used_hint_source(HintSource* hint_source) { _used_hint_sources.emplace_back(hint_source); }

    [[nodiscard]] WorldNode* spawn_node() const { return _spawn_node; }
    [[nodiscard]] WorldNode* end_node() const { return _nodes.at("end"); }

    [[nodiscard]] WorldRegion* dark_region() const { return _dark_region; }
    void dark_region(WorldRegion* region);

    [[nodiscard]] const std::vector<EntityType*>& fahl_enemies() const { return _fahl_enemies; }
    void add_fahl_enemy(EntityType* enemy) { _fahl_enemies.emplace_back(enemy); }

    [[nodiscard]] const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& teleport_tree_pairs() const { return _teleport_tree_pairs; }
    void teleport_tree_pairs(const std::vector<std::pair<WorldTeleportTree*, WorldTeleportTree*>>& new_pairs) { _teleport_tree_pairs = new_pairs; }

    void add_paths_for_tree_connections(bool require_tibor_access);

    void load_model_from_json();

private:
    void load_additional_item_data();
    void load_item_sources();
    void load_nodes();
    void load_paths();
    void load_regions();
    void load_spawn_locations();
    void load_hint_sources();
    void init_item_distributions();
    void load_teleport_trees();
};
