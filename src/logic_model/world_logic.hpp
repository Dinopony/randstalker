#pragma once

#include <map>
#include <vector>
#include <string>

class World;
class WorldNode;
class WorldPath;
class WorldRegion;

class WorldLogic {
private:
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;

    WorldNode* _spawn_node = nullptr;

public:
    explicit WorldLogic(const World& world);
    ~WorldLogic();

    [[nodiscard]] const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    [[nodiscard]] WorldNode* node(const std::string& id) const { return _nodes.at(id); }

    void spawn_node(WorldNode* node) { _spawn_node = node; }
    [[nodiscard]] WorldNode* spawn_node() const { return _spawn_node; }
    [[nodiscard]] WorldNode* end_node() const { return _nodes.at("end"); }

    [[nodiscard]] const std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    [[nodiscard]] const std::vector<WorldRegion*>& regions() const { return _regions; }
    [[nodiscard]] WorldRegion* region(const std::string& name) const;

private:
    void load_nodes(const World& world);
    void load_paths(const World& world);
    void load_regions();
};