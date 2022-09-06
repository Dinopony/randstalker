#pragma once

#include <utility>
#include <vector>

#include "item_source.hpp"

#include "world_path.hpp"

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
    WorldRegion* _region = nullptr;

public:
    WorldNode(std::string id, std::string name, std::vector<std::string> hints = {}) :
        _id             (std::move(id)),
        _name           (std::move(name)),
        _hints          (std::move(hints))
    {}

    [[nodiscard]] const std::string& id() const { return _id; }

    [[nodiscard]] const std::string& name() const { return _name; }
    void name(const std::string& name) { _name = name; }

    [[nodiscard]] const std::vector<ItemSource*>& item_sources() const { return _item_sources; }
    void add_item_source(ItemSource* source) {  _item_sources.emplace_back(source); }

    [[nodiscard]] const std::vector<WorldPath*>& outgoing_paths() const { return _outgoing_paths; }
    void add_outgoing_path(WorldPath* path) { _outgoing_paths.emplace_back(path); }

    [[nodiscard]] const std::vector<WorldPath*>& ingoing_paths() const { return _ingoing_paths; }
    void add_ingoing_path(WorldPath* path) { _ingoing_paths.emplace_back(path); }

    [[nodiscard]] const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.emplace_back(hint); }

    [[nodiscard]] WorldRegion* region() const { return _region; }
    void region(WorldRegion* region) { _region = region; }

    [[nodiscard]] Json to_json() const
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
