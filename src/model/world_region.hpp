#pragma once

#include <vector>
#include "../tools/unsorted_set.hpp"
#include "world_path.hpp"
#include "item_source.hpp"

class WorldRegion
{
private:
    std::string _id;
    std::string _name;
    std::vector<ItemSource*> _item_sources;
    std::vector<WorldPath*> _outgoing_paths;
    std::vector<WorldPath*> _ingoing_paths;    
    std::vector<std::string> _hints;
    std::vector<uint16_t> _dark_map_ids;

public:
    WorldRegion(const std::string& id, const std::string& name, const std::vector<std::string>& hints = {}, const std::vector<uint16_t> dark_map_ids = {}) :
        _id           (id),
        _name         (name),
        _item_sources (),
        _hints        (hints),
        _dark_map_ids (dark_map_ids)
    {}

    const std::string& id() const { return _id; }

    const std::string& name() const { return _name; }
    WorldRegion& name(const std::string& name) { _name = name; return *this;}

    std::vector<ItemSource*> item_sources() const { return _item_sources; }
    void add_item_source(ItemSource* source) {  _item_sources.push_back(source); }

    const std::vector<WorldPath*>& outgoing_paths() const { return _outgoing_paths; }
    void add_outgoing_path(WorldPath* path) { _outgoing_paths.push_back(path); }

    const std::vector<WorldPath*>& ingoing_paths() const { return _ingoing_paths; }
    void add_ingoing_path(WorldPath* path) { _ingoing_paths.push_back(path); }

    const std::vector<std::string>& hints() const { return _hints; }
    void add_hint(const std::string& hint) { _hints.push_back(hint); }
    
    const std::vector<uint16_t>& dark_map_ids() const { return _dark_map_ids; }

    
    void dark_rooms(const std::vector<uint16_t>& darkRooms) { _dark_map_ids = darkRooms; }
    void dark_rooms(uint16_t lowerBound, uint16_t upperBound)
    {
        _dark_map_ids.clear();
        for (uint16_t i = lowerBound; i <= upperBound; ++i)
            _dark_map_ids.push_back(i);
    }

    Json to_json() const
    {
        Json json;
        json["id"] = _id;
        json["name"] = _name;
        if(!_hints.empty())
            json["hints"] = _hints;
        if(!_dark_map_ids.empty())
            json["dark_map_ids"] = _dark_map_ids;
        return json;
    }

    static WorldRegion* from_json(const Json& json)
    {
        const std::string& id = json.at("id");
        const std::string& name = json.at("name");

        std::vector<std::string> hints;
        if(json.contains("hints")) 
            json.at("hints").get_to(hints);

        std::vector<uint16_t> dark_map_ids;
        if(json.contains("dark_map_ids")) 
            json.at("dark_map_ids").get_to(dark_map_ids);

        return new WorldRegion(id, name, hints, dark_map_ids);
    }
};