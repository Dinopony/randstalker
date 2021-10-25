#pragma once

#include <vector>
#include <string>
#include "../extlibs/json.hpp"
#include "world_region.hpp"

class HintSource 
{
public:
    HintSource(const std::vector<uint16_t>& text_ids, const std::string& description, 
            WorldRegion* region, bool special, bool small_textbox) :
        _text_ids       (text_ids),
        _description    (description),
        _region         (region),
        _special        (special),
        _small_textbox  (small_textbox),
        _text           ()
    {}

    const std::vector<uint16_t>& text_ids() const { return _text_ids; }
    const std::string& description() const { return _description; }
    WorldRegion* region() const { return _region; }
    bool special() const { return _special; }
    bool small_textbox() const { return _small_textbox; }

    const std::string& text() const { return _text; }
    void text(const std::string& text) { _text = text; }

    Json to_json() const
    {
        Json json;
        json["description"] = _description;
        json["textIds"] = _text_ids;

        if(_special)
            json["special"] = true;
        else
            json["regionId"] = _region->id();
        
        if(_small_textbox)
            json["smallTextbox"] = _small_textbox;

        return json;
    }

    static HintSource* from_json(const Json& json, const std::map<std::string, WorldRegion*>& regions)
    {
        std::string description = json.at("description");
        std::vector<uint16_t> text_ids = json.at("textIds");
        bool special = json.value("special", false);
        bool small_textbox = json.value("smallTextbox", false);

        WorldRegion* region = nullptr;
        std::string region_id = json.value("regionId", "");
        if(!region_id.empty())
            region = regions.at(region_id);

        return new HintSource(text_ids, description, region, special, small_textbox);
    }

private:
    std::vector<uint16_t> _text_ids;
    std::string _description;
    WorldRegion* _region;
    bool _special;
    bool _small_textbox;
    std::string _text;
};