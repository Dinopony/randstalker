#pragma once

#include <vector>
#include "../tools/unsorted_set.hpp"
#include "world_path.hpp"
#include "item_source.hpp"

class WorldRegion;

class WorldNode
{
private:
    std::string _id;
    std::string _name;
    std::vector<ItemSource*> _item_sources;
    std::vector<WorldPath*> _outgoing_paths;
    std::vector<WorldPath*> _ingoing_paths;    
    std::vector<std::string> _hints;
    WorldRegion* _region;

public:
    WorldNode(const std::string& id, const std::string& name, const std::vector<std::string>& hints = {}) :
        _id             (id),
        _name           (name),
        _item_sources   (),
        _hints          (hints),
        _region         (nullptr)
    {}

    const std::string& id() const { return _id; }

    const std::string& name() const { return _name; }
    WorldNode& name(const std::string& name) { _name = name; return *this;}

    const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    void add_item_source(ItemSource* source) {  _item_sources.push_back(source); }

    const std::vector<WorldPath*>& outgoing_paths() const { return _outgoing_paths; }
    void add_outgoing_path(WorldPath* path) { _outgoing_paths.push_back(path); }

    const std::vector<WorldPath*>& ingoing_paths() const { return _ingoing_paths; }
    void add_ingoing_path(WorldPath* path) { _ingoing_paths.push_back(path); }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.push_back(hint); }

    WorldRegion* region() const { return _region; }
    void region(WorldRegion* region) { _region = region; }

    Json to_json() const
    {
        Json json;
        json["name"] = _name;
        if(!_hints.empty())
            json["hints"] = _hints;

        return json;
    }

    static WorldNode* from_json(const std::string& id, const Json& json)
    {
        const std::string& name = json.at("name");

        std::vector<std::string> hints;
        if(json.contains("hints")) 
            json.at("hints").get_to(hints);

        return new WorldNode(id, name, hints);
    }
};