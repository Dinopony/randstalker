#pragma once

#include <map>
#include <vector>
#include <string>

class World;
class WorldNode;
class WorldPath;
class WorldRegion;
class HintSource;
class SpawnLocation;
class ItemDistribution;

class WorldLogic {
private:
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;
    std::map<std::string, HintSource*> _hint_sources;
    std::map<std::string, SpawnLocation*> _spawn_locations;
    std::map<uint8_t, ItemDistribution*> _item_distributions;

    WorldNode* _spawn_node = nullptr;
    WorldRegion* _dark_region = nullptr;

public:
    explicit WorldLogic(const World& world);
    ~WorldLogic();

    [[nodiscard]] const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    [[nodiscard]] WorldNode* node(const std::string& id) const { return _nodes.at(id); }

    [[nodiscard]] const std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    [[nodiscard]] const std::vector<WorldRegion*>& regions() const { return _regions; }
    [[nodiscard]] WorldRegion* region(const std::string& name) const;

    [[nodiscard]] const std::map<std::string, SpawnLocation*>& spawn_locations() const { return _spawn_locations; }
    void add_spawn_location(SpawnLocation* spawn);

    [[nodiscard]] const std::map<uint8_t, ItemDistribution*>& item_distributions() const { return _item_distributions; }
    [[nodiscard]] ItemDistribution* item_distribution(uint8_t item_id) const { return _item_distributions.at(item_id); }
    [[nodiscard]] std::vector<uint8_t> build_mandatory_items_vector() const;
    [[nodiscard]] std::vector<uint8_t> build_filler_items_vector() const;

    [[nodiscard]] const std::map<std::string, HintSource*>& hint_sources() const { return _hint_sources; }
    [[nodiscard]] HintSource* hint_source(const std::string& name) const { return _hint_sources.at(name); }

    void active_spawn_location(SpawnLocation* spawn, World& world);
    [[nodiscard]] WorldNode* spawn_node() const { return _spawn_node; }
    [[nodiscard]] WorldNode* end_node() const { return _nodes.at("end"); }

    [[nodiscard]] WorldRegion* dark_region() const { return _dark_region; }
    void dark_region(WorldRegion* region, World& world);

private:
    void load_nodes(const World& world);
    void load_paths(const World& world);
    void load_regions();
    void load_spawn_locations();
    void load_hint_sources();
    void load_item_distributions();
};
