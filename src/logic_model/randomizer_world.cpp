#include "randomizer_world.hpp"

#include "world_teleport_tree.hpp"
#include "spawn_location.hpp"
#include <landstalker-lib/model/world.hpp>
#include <landstalker-lib/tools/game_text.hpp>
#include <landstalker-lib/exceptions.hpp>
#include "world_node.hpp"
#include "world_path.hpp"
#include "world_region.hpp"
#include "hint_source.hpp"

// Include headers automatically generated from model json files
#include "data/item.json.hxx"
#include "data/item_source.json.hxx"
#include "data/world_node.json.hxx"
#include "data/world_path.json.hxx"
#include "data/world_region.json.hxx"
#include "data/spawn_location.json.hxx"
#include "data/hint_source.json.hxx"
#include "data/world_teleport_tree.json.hxx"
#include "landstalker-lib/tools/stringtools.hpp"

#include <iostream>
#include <regex>

RandomizerWorld::RandomizerWorld() :
    World()
{
    // Add "standard" gold items which can be used through item distribution
    this->add_item(new ItemGolds(ITEM_1_GOLD, 1));
    this->add_item(new ItemGolds(ITEM_20_GOLDS, 20));
    this->add_item(new ItemGolds(ITEM_50_GOLDS, 50));
    this->add_item(new ItemGolds(ITEM_100_GOLDS, 100));
    this->add_item(new ItemGolds(ITEM_200_GOLDS, 200));
}

RandomizerWorld::~RandomizerWorld()
{
    for (ItemSource* source : _item_sources)
        delete source;
    for (auto& [key, node] : _nodes)
        delete node;
    for (WorldPath* path : _paths)
        delete path;
    for (WorldRegion* region : _regions)
        delete region;
    for (HintSource* hint_source : _hint_sources)
        delete hint_source;
    for (auto& [key, spawn_loc] : _available_spawn_locations)
        delete spawn_loc;
    for (auto& [tree_1, tree_2] : _teleport_tree_pairs)
    {
        if(tree_1 != tree_2)
            delete tree_2;
        delete tree_1;
    }
    for(Item* item : _archipelago_items)
        delete item;
}

ItemSource* RandomizerWorld::item_source(const std::string& name) const
{
    for (ItemSource* source : _item_sources)
        if (source->name() == name)
            return source;

    throw std::out_of_range("No source with given name");
}

std::vector<ItemSource*> RandomizerWorld::item_sources_with_item(Item* item)
{
    std::vector<ItemSource*> sources_with_item;

    for (ItemSource* source : _item_sources)
        if (source->item() == item)
            sources_with_item.emplace_back(source);

    return sources_with_item;
}

std::map<std::string, uint8_t> RandomizerWorld::item_names(bool strict) const
{
    // Add factual item names from the world data
    std::map<std::string, uint8_t> item_names;
    for(uint8_t i=0 ; i<ITEM_COUNT ; ++i)
    {
        try
        {
            item_names[this->item(i)->name()] = i;
        }
        catch(std::out_of_range&)
        {
            item_names["No" + std::to_string(i)] = i;
        }
    }

    // Add alternative names which depend on settings unless we are in "strict" mode
    if(!strict)
    {
        item_names["Kazalt Jewel"] = ITEM_RED_JEWEL;
        item_names["Yellow Jewel"] = ITEM_YELLOW_JEWEL;
        item_names["Blue Jewel"] = ITEM_BLUE_JEWEL;
        item_names["Green Jewel"] = ITEM_GREEN_JEWEL;
    }

    return item_names;
}

void RandomizerWorld::load_item_sources(bool lite_mode)
{
    Json item_sources_json = Json::parse(ITEM_SOURCES_JSON);
    for(const Json& source_json : item_sources_json)
    {
        _item_sources.emplace_back(ItemSource::from_json(source_json, *this, !lite_mode));
    }

    // The following chests are absent from the game on release or modded out of the game for the rando, and their IDs are therefore free:
    // 0x0E (14): Mercator Kitchen (variant?)
    // 0x1E (30): King Nole's Cave spiral staircase (variant with enemies?) ---> 29 is the one used in rando
    // 0x20 (32): Boulder chase hallway (variant with enemies?) ---> 31 is the one used in rando
    // 0x25 (37): Thieves Hideout entrance (variant with water)
    // 0x27 (39): Thieves Hideout entrance (waterless variant)
    // 0x28 (40): Thieves Hideout entrance (waterless variant)
    // 0x33 (51): Thieves Hideout second room (waterless variant)
    // 0x3D (61): Thieves Hideout reward room (Kayla cutscene variant)
    // 0x3E (62): Thieves Hideout reward room (Kayla cutscene variant)
    // 0x3F (63): Thieves Hideout reward room (Kayla cutscene variant)
    // 0x40 (64): Thieves Hideout reward room (Kayla cutscene variant)
    // 0x41 (65): Thieves Hideout reward room (Kayla cutscene variant)
    // 0xBB (187): Crypt (Larson. E room)
    // 0xBC (188): Crypt (Larson. E room)
    // 0xBD (189): Crypt (Larson. E room)
    // 0xBE (190): Crypt (Larson. E room)
    // 0xC3 (195): Map 712 / 0x2C8 ???
}

WorldPath* RandomizerWorld::path(WorldNode* origin, WorldNode* destination)
{
    for(WorldPath* path : _paths)
        if(path->origin() == origin && path->destination() == destination)
            return path;
    return nullptr;
}

WorldPath* RandomizerWorld::path(const std::string& origin_name, const std::string& destination_name)
{
    WorldNode* origin = _nodes.at(origin_name);
    WorldNode* destination = _nodes.at(destination_name);
    return this->path(origin, destination);
}

void RandomizerWorld::add_path(WorldPath* path)
{
    _paths.emplace_back(path);
}

WorldRegion* RandomizerWorld::region(const std::string& name) const
{
    for(WorldRegion* region : _regions)
        if(region->name() == name)
            return region;
    return nullptr;
}

std::vector<std::string> RandomizerWorld::spawn_location_names() const
{
    std::vector<std::string> ret;
    for(auto& [name, _] : _available_spawn_locations)
        ret.emplace_back(name);
    return ret;
}

void RandomizerWorld::add_spawn_location(SpawnLocation* spawn)
{
    _available_spawn_locations[spawn->id()] = spawn;
}

void RandomizerWorld::dark_region(WorldRegion* region)
{
    _dark_region = region;
    this->dark_maps(region->dark_map_ids());

    for(WorldNode* node : region->nodes())
    {
        for (WorldPath* path : node->ingoing_paths())
            if(path->origin()->region() != region)
                path->add_required_item(this->item(ITEM_LANTERN));

        node->add_hint("in a very dark place");
    }
}

void RandomizerWorld::add_custom_dialogue(Entity* entity, const std::string& text)
{
    _custom_dialogues[entity] = GameText(text).get_output();
}

void RandomizerWorld::add_custom_dialogue_raw(Entity* entity, const std::string& text)
{
    _custom_dialogues[entity] = text;
}

void RandomizerWorld::add_paths_for_tree_connections(bool require_tibor_access)
{
    std::vector<WorldNode*> required_nodes;
    if(require_tibor_access)
        required_nodes = { this->node("tibor") };

    for(auto& pair : _teleport_tree_pairs)
    {
        WorldNode* first_node = this->node(pair.first->node_id());
        WorldNode* second_node = this->node(pair.second->node_id());
        this->add_path(new WorldPath(first_node, second_node, 1, {}, required_nodes));
        this->add_path(new WorldPath(second_node, first_node, 1, {}, required_nodes));
    }
}

Item* RandomizerWorld::add_archipelago_item(std::string item_name, std::string player_name, bool use_shop_naming)
{
    const std::set<char> VOWELS = { 'a', 'e', 'i', 'o', 'u', 'y', 'A', 'E', 'I', 'O', 'U', 'Y' };

    player_name.erase(std::remove(player_name.begin(), player_name.end(), '_'), player_name.end());
    // Replace underscores which don't exist in LS by hyphens
    std::replace(item_name.begin(), item_name.end(), '_', '-');
    // Replace large " - " by simple spaces, we sadly don't have enough space for such luxury
    item_name = std::regex_replace(item_name, std::regex(" - "), " ");
    // Remove parentheses which doesn't contain a number ("Arrows (10)" is okay while "Limbo (E3M7)" is not)
    item_name = std::regex_replace(item_name, std::regex(R"(\(\D(.*)\))"), "");
    // Remove successive spaces
    item_name = std::regex_replace(item_name, std::regex(R"(\s(\s+))"), " ");

    size_t max_full_string_size = (use_shop_naming) ? 30 : 38;

    // Use all the remaining space for item name
    size_t max_item_name_size = max_full_string_size - player_name.size();

    // If the item name is too long, try truncating individual words to keep the global meaning
    std::string shortened_item_name = item_name;
    std::vector<std::string> words = stringtools::split_with_delims(shortened_item_name, { ' ', '-', '(', ')' });
    size_t current_word_index = 0;
    while(shortened_item_name.size() > max_item_name_size && current_word_index < words.size())
    {
        std::string& current_word = words[current_word_index];
        if(current_word.size() > 4)
        {
            for(size_t i = 1 ; i < current_word.size() - 3 ; ++i)
            {
                if(VOWELS.contains(current_word[i]) && !VOWELS.contains(current_word[i + 1]))
                {
                    current_word = current_word.substr(0, i+2);
                    if(current_word_index + 1 < words.size() && words[current_word_index + 1] == " ")
                    {
                        current_word += ".";
                        words.erase(words.begin() + ((int) current_word_index + 1));
                    }
                    break;
                }
            }
        }

        ++current_word_index;
        shortened_item_name = stringtools::join(words, "");
    }

    // If after shortening individual words, the global name is still too long, just truncate it
    if(shortened_item_name.size() > max_item_name_size)
        shortened_item_name = shortened_item_name.substr(0, max_item_name_size-1) + ".";

    std::string formatted_name;
    if(use_shop_naming)
        formatted_name = player_name + "'s " + shortened_item_name;
    else
        formatted_name = shortened_item_name + " to " + player_name;

    for(Item* item : _archipelago_items)
        if(item->name() == formatted_name)
            return item;

    Item* new_item = new Item(ITEM_ARCHIPELAGO, formatted_name, 0, 0, 0, 0);
    _archipelago_items.emplace_back(new_item);
    return new_item;
}

void RandomizerWorld::spawn_location(const SpawnLocation* spawn)
{
    _spawn_location = spawn;

    this->spawn_map_id(spawn->map_id());
    this->spawn_position_x(spawn->position_x());
    this->spawn_position_y(spawn->position_y());
    this->spawn_orientation(spawn->orientation());

    if(this->starting_life() == 0)
        this->starting_life(spawn->starting_life());
}

HintSource* RandomizerWorld::hint_source(const std::string& name) const
{
    for(HintSource* source : _hint_sources)
        if(source->description() == name)
            return source;

    throw LandstalkerException("Could not find hint source '" + name + "' as requested");
}

void RandomizerWorld::load_model_from_json(bool lite_mode)
{
    this->load_additional_item_data();
    this->load_item_sources(lite_mode);
    this->load_nodes();
    this->load_paths();
    this->load_regions();
    this->load_spawn_locations();
    this->load_hint_sources();
    this->load_teleport_trees();
}

void RandomizerWorld::load_nodes()
{
    Json nodes_json = Json::parse(WORLD_NODES_JSON);
    for(auto& [node_id, node_json] : nodes_json.items())
    {
        WorldNode* new_node = WorldNode::from_json(node_id, node_json);
        _nodes[node_id] = new_node;
    }

    for(ItemSource* source : this->item_sources())
    {
        const std::string& node_id = source->node_id();
        try {
            _nodes.at(node_id)->add_item_source(source);
        } catch(std::out_of_range&) {
            throw LandstalkerException("Could not find node '" + node_id + "' referenced by item source '" + source->name() + "'");
        }
    }
}

void RandomizerWorld::load_paths()
{
    Json paths_json = Json::parse(WORLD_PATHS_JSON);
    for(const Json& path_json : paths_json)
    {
        this->add_path(WorldPath::from_json(path_json, _nodes, this->items()));

        if(path_json.contains("twoWay") && path_json.at("twoWay"))
        {
            Json inverted_json = path_json;
            inverted_json["fromId"] = path_json.at("toId");
            inverted_json["toId"] = path_json.at("fromId");
            this->add_path(WorldPath::from_json(inverted_json, _nodes, this->items()));
        }
    }
}

void RandomizerWorld::load_regions()
{
    Json regions_json = Json::parse(WORLD_REGIONS_JSON);
    for(const Json& region_json : regions_json)
        _regions.emplace_back(WorldRegion::from_json(region_json, _nodes));

    for(auto& [id, node] : _nodes)
        if(node->region() == nullptr)
            throw LandstalkerException("Node '" + node->id() + "' doesn't belong to any region");
}

void RandomizerWorld::load_spawn_locations()
{
    // Load base model
    Json spawns_json = Json::parse(SPAWN_LOCATIONS_JSON);
    for(auto& [id, spawn_json] : spawns_json.items())
        this->add_spawn_location(SpawnLocation::from_json(id, spawn_json));
}

void RandomizerWorld::load_hint_sources()
{
    Json hint_sources_json = Json::parse(HINT_SOURCES_JSON, nullptr, true, true);
    for(const Json& hint_source_json : hint_sources_json)
    {
        HintSource* new_source = HintSource::from_json(hint_source_json, _nodes);
        _hint_sources.emplace_back(new_source);
    }
}

void RandomizerWorld::load_teleport_trees()
{
    Json trees_json = Json::parse(WORLD_TELEPORT_TREES_JSON);
    for(const Json& tree_pair_json : trees_json)
    {
        WorldTeleportTree* tree_1 = WorldTeleportTree::from_json(tree_pair_json[0]);
        WorldTeleportTree* tree_2 = WorldTeleportTree::from_json(tree_pair_json[1]);
        tree_1->paired_map_id(tree_2->map_id());
        tree_2->paired_map_id(tree_1->map_id());
        _teleport_tree_pairs.emplace_back(std::make_pair(tree_1, tree_2));
    }
}

void RandomizerWorld::load_additional_item_data()
{
    Json items_json = Json::parse(ITEMS_JSON);
    for(auto& [id_string, item_json] : items_json.items())
    {
        uint8_t id = std::stoi(id_string);
        this->item(id)->apply_json(item_json);
    }
}