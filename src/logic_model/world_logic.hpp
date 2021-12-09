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

class WorldLogic {
private:
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;
    std::map<std::string, HintSource*> _hint_sources;
    std::map<std::string, SpawnLocation*> _spawn_locations;
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

    [[nodiscard]] const std::map<std::string, HintSource*>& hint_sources() const { return _hint_sources; }
    [[nodiscard]] HintSource* hint_source(const std::string& name) const { return _hint_sources.at(name); }

    void spawn_node(WorldNode* node) { _spawn_node = node; }
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
};
