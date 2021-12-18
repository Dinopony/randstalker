#pragma once

#include <map>
#include <vector>
#include <string>
#include <landstalker_lib/model/world.hpp>

class WorldNode;
class WorldPath;
class WorldRegion;
class HintSource;
class ItemDistribution;

class RandomizerWorld : public World {
private:
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;
    std::map<std::string, SpawnLocation*> _available_spawn_locations;
    std::map<uint8_t, ItemDistribution*> _item_distributions;

    std::vector<HintSource*> _hint_sources;
    std::vector<HintSource*> _used_hint_sources;

    WorldNode* _spawn_node = nullptr;
    WorldRegion* _dark_region = nullptr;

public:
    explicit RandomizerWorld(const md::ROM& rom);
    ~RandomizerWorld();

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

    [[nodiscard]] const std::map<uint8_t, ItemDistribution*>& item_distributions() const { return _item_distributions; }
    [[nodiscard]] ItemDistribution* item_distribution(uint8_t item_id) const { return _item_distributions.at(item_id); }
    [[nodiscard]] std::map<uint8_t, uint16_t> item_quantities() const;

    [[nodiscard]] const std::vector<HintSource*>& hint_sources() const { return _hint_sources; }
    [[nodiscard]] HintSource* hint_source(const std::string& name) const;
    [[nodiscard]] const std::vector<HintSource*>& used_hint_sources() const { return _used_hint_sources; }
    void add_used_hint_source(HintSource* hint_source) { _used_hint_sources.emplace_back(hint_source); }

    [[nodiscard]] WorldNode* spawn_node() const { return _spawn_node; }
    [[nodiscard]] WorldNode* end_node() const { return _nodes.at("end"); }

    [[nodiscard]] WorldRegion* dark_region() const { return _dark_region; }
    void dark_region(WorldRegion* region);

private:
    void load_nodes();
    void load_paths();
    void load_regions();
    void load_spawn_locations();
    void load_hint_sources();
    void load_item_distributions();
};
