#pragma once

#include <map>
#include <vector>
#include <string>

class World;
class WorldNode;
class WorldPath;
class WorldRegion;
class Item;
class RandomizerOptions;

class WorldLogic {
private:
    std::map<std::string, WorldNode*> _nodes;
    std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*> _paths;
    std::vector<WorldRegion*> _regions;

    WorldNode* _spawn_node;

public:
    WorldLogic(const World& world, const RandomizerOptions& options);
    ~WorldLogic();

    const std::map<std::string, WorldNode*>& nodes() const { return _nodes; }
    WorldNode* node(const std::string& id) const { return _nodes.at(id); }

    void spawn_node(WorldNode* node) { _spawn_node = node; }
    WorldNode* spawn_node() const { return _spawn_node; }
    WorldNode* end_node() const { return _nodes.at("end"); }

    WorldNode* first_node_with_item(Item* item);

    const std::map<std::pair<WorldNode*, WorldNode*>, WorldPath*>& paths() const { return _paths; }
    WorldPath* path(WorldNode* origin, WorldNode* destination);
    WorldPath* path(const std::string& origin_name, const std::string& destination_name);
    void add_path(WorldPath* path);

    const std::vector<WorldRegion*> regions() const { return _regions; }
    WorldRegion* region(const std::string& name) const;

private:
    void init_nodes(const World& world);
    void init_paths(const World& world, const RandomizerOptions& options);
    void init_regions();
};