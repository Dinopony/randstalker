#include "world.hpp"

#include <iostream>

#include "tools/textbanks_decoder.hpp"
#include "tools/textbanks_encoder.hpp"

#include "model/entity_type.hpp"
#include "model/map.hpp"
#include "model/item.hpp"
#include "model/item_source.hpp"
#include "model/map.hpp"
#include "model/world_node.hpp"
#include "model/world_path.hpp"
#include "model/spawn_location.hpp"
#include "model/hint_source.hpp"
#include "model/world_region.hpp"
#include "model/world_teleport_tree.hpp"

#include "exceptions.hpp"
#include "world_solver.hpp"
#include "randomizer_options.hpp"
#include "offsets.hpp"

// Include headers automatically generated from model json files
#include "model/entity_type.json.hxx"
#include "model/item.json.hxx"
#include "model/world_node.json.hxx"
#include "model/world_path.json.hxx"
#include "model/item_source.json.hxx"
#include "model/spawn_location.json.hxx"
#include "model/hint_source.json.hxx"
#include "model/world_region.json.hxx"
#include "model/world_teleport_tree.json.hxx"
#include "assets/game_strings.json.hxx"

World::World(const md::ROM& rom, const RandomizerOptions& options) :
    _options                (options),
    _active_spawn_location  (nullptr),
    _dark_region            (nullptr)
{
    this->init_items();
    this->init_entity_types(rom);
    this->init_maps(rom);
    this->init_nodes();
    this->init_item_sources();
    this->init_paths();
    this->init_regions();
    this->init_spawn_locations();
    this->init_teleport_trees();
    this->init_game_strings(rom);
    this->init_hint_sources();

}

World::~World()
{
    for (auto& [key, item] : _items)
        delete item;
    for (auto& [key, node] : _nodes)
        delete node;
    for (auto& [key, path] : _paths)
        delete path;
    for (ItemSource* source : _item_sources)
        delete source;
    for (auto& [key, spawn_loc] : _spawn_locations)
        delete spawn_loc;
    for (auto& [key, hint_source] : _hint_sources)
        delete hint_source;
    for (WorldRegion* region : _regions)
        delete region;
    for (auto& [tree_1, tree_2] : _teleport_tree_pairs)
    {
        delete tree_1;
        delete tree_2;
    }
    for (auto& [id, entity] : _entity_types)
        delete entity;
}

Item* World::item(const std::string& name) const
{
    if(name.empty())
        return nullptr;

    for (auto& [key, item] : _items)
        if(item->name() == name)
            return item;

    return nullptr;
}

Item* World::add_item(Item* item)
{ 
    _items[item->id()] = item;
    return item;
}

Item* World::add_gold_item(uint8_t worth)
{
    uint8_t highest_item_id = _items.rbegin()->first;

    // Try to find an item with the same worth
    for(uint8_t i=ITEM_GOLDS_START ; i<=highest_item_id ; ++i)
        if(_items[i]->gold_value() == worth)
            return _items[i];

    // If we consumed all item IDs, don't add it you fool!
    if(highest_item_id == 0xFF)
        return nullptr;

    return this->add_item(new ItemGolds(highest_item_id+1, worth));
}

std::vector<Item*> World::starting_inventory() const
{
    std::vector<Item*> starting_inventory;
    for(auto& [id, item] : _items)
    {
        uint8_t item_starting_quantity = item->starting_quantity();
        for(uint8_t i=0 ; i<item_starting_quantity ; ++i)
            starting_inventory.push_back(item);
    }
    return starting_inventory;
}

WorldNode* World::first_node_with_item(Item* item)
{
    for (auto& [key, node] : _nodes)
    {
        std::vector<ItemSource*> sources = node->item_sources();
        for (ItemSource* source : sources)
            if (source->item() == item)
                return node;
    }

    return nullptr;
}


WorldPath* World::path(WorldNode* origin, WorldNode* destination)
{
    return _paths.at(std::make_pair(origin, destination));
}

WorldPath* World::path(const std::string& origin_name, const std::string& destination_name)
{
    WorldNode* origin = _nodes.at(origin_name);
    WorldNode* destination = _nodes.at(destination_name);
    return this->path(origin, destination);
}

void World::add_path(WorldPath* path)
{
    _paths[std::make_pair(path->origin(), path->destination())] = path;
}


std::vector<ItemSource*> World::item_sources_with_item(Item* item)
{
    std::vector<ItemSource*> sources_with_item;

    for (auto& [key, node] : _nodes)
    {
        std::vector<ItemSource*> sources = node->item_sources();
        for (ItemSource* source : sources)
            if (source->item() == item)
                sources_with_item.push_back(source);
    }

    return sources_with_item;
}

void World::add_spawn_location(SpawnLocation* spawn)
{
    _spawn_locations[spawn->id()] = spawn;
}

void World::init_items()
{
    // Load base model
    Json items_json = Json::parse(ITEMS_JSON);
    for(auto& [id_string, item_json] : items_json.items())
    {
        uint8_t id = std::stoi(id_string);
        this->add_item(Item::from_json(id, item_json));
    }
    std::cout << _items.size() << " items loaded." << std::endl;

    // Alter a few things depending on settings
    if (_options.use_armor_upgrades())
    {
        _items[ITEM_STEEL_BREAST]->gold_value(250);
        _items[ITEM_CHROME_BREAST]->gold_value(250);
        _items[ITEM_SHELL_BREAST]->gold_value(250);
        _items[ITEM_HYPER_BREAST]->gold_value(250);
    }

    if (_options.consumable_record_book())
    {
        _items[ITEM_RECORD_BOOK]->max_quantity(9);
        uint16_t currentPrice = _items[ITEM_RECORD_BOOK]->gold_value();
        _items[ITEM_RECORD_BOOK]->gold_value(currentPrice / 5);
    }

    if(_options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        _items[ITEM_RED_JEWEL]->name("Kazalt Jewel");
        _items[ITEM_RED_JEWEL]->allowed_on_ground(false);
        _items[ITEM_RED_JEWEL]->max_quantity(_options.jewel_count());
    }

    // Patch model if user specified a model patch
    const Json& model_patch = _options.items_model_patch();
    for(auto& [id_string, patch_json] : model_patch.items())
    {
        uint8_t id = std::stoi(id_string);
        if(_items.count(id))
            _items.at(id)->apply_json(patch_json);
        else
            this->add_item(Item::from_json(id, patch_json));
    }

    // Process custom starting quantities for items
    const std::map<std::string, uint8_t>& starting_items = _options.starting_items();
    for(auto& [item_name, quantity] : starting_items)
    {
        Item* item = this->item(item_name);
        if(!item)
        {
            std::stringstream msg;
            msg << "Cannot set starting quantity of unknown item '" << item_name << "'";
            throw RandomizerException(msg.str());
        }

        item->starting_quantity(std::min<uint8_t>(quantity, 9));
    }
}

void World::init_item_sources()
{
    Json item_sources_json = Json::parse(ITEM_SOURCES_JSON);
    for(const Json& source_json : item_sources_json)
    {
        _item_sources.push_back(ItemSource::from_json(source_json, *this));
    }
    std::cout << _item_sources.size() << " item sources loaded." << std::endl;

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

void World::init_nodes()
{
    Json nodes_json = Json::parse(WORLD_NODES_JSON);
    for(auto& [node_id, node_json] : nodes_json.items())
    {
        WorldNode* new_node = WorldNode::from_json(node_id, node_json);
        _nodes[node_id] = new_node;
    }
    std::cout << _nodes.size() << " nodes loaded." << std::endl;
}

void World::init_paths()
{
    Json paths_json = Json::parse(WORLD_PATHS_JSON);
    for(const Json& path_json : paths_json)
    {
        this->add_path(WorldPath::from_json(path_json, _nodes, _items));

        if(path_json.contains("twoWay") && path_json.at("twoWay"))
        {
            Json inverted_json = path_json;
            inverted_json["fromId"] = path_json.at("toId");
            inverted_json["toId"] = path_json.at("fromId");
            this->add_path(WorldPath::from_json(inverted_json, _nodes, _items));
        }
    }
    std::cout << _paths.size() << " paths loaded." << std::endl;   

    // Determine the list of required jewels to go from King Nole's Cave to Kazalt depending on settings
    WorldPath* path_to_kazalt = this->path("king_nole_cave", "kazalt");
    if(_options.jewel_count() > MAX_INDIVIDUAL_JEWELS)
    {
        for(int i=0; i<_options.jewel_count() ; ++i)
            path_to_kazalt->add_required_item(_items[ITEM_RED_JEWEL]);
    }
    else if(_options.jewel_count() >= 1)
    {
        path_to_kazalt->add_required_item(_items[ITEM_RED_JEWEL]);
        if(_options.jewel_count() >= 2)
            path_to_kazalt->add_required_item(_items[ITEM_PURPLE_JEWEL]);
        if(_options.jewel_count() >= 3)
            path_to_kazalt->add_required_item(_items[ITEM_GREEN_JEWEL]);
        if(_options.jewel_count() >= 4)
            path_to_kazalt->add_required_item(_items[ITEM_BLUE_JEWEL]);
        if(_options.jewel_count() >= 5)
            path_to_kazalt->add_required_item(_items[ITEM_YELLOW_JEWEL]);
    }

    if(_options.remove_gumi_boulder())
    {
        this->add_path(new WorldPath(_nodes.at("route_gumi_ryuma"), _nodes.at("gumi")));
    }

    // Handle paths related to specific tricks
    if(_options.handle_ghost_jumping_in_logic())
    {
        this->add_path(new WorldPath(_nodes.at("route_lake_shrine"), _nodes.at("route_lake_shrine_cliff")));
    }
}

void World::init_regions()
{
    Json regions_json = Json::parse(WORLD_REGIONS_JSON);
    for(const Json& region_json : regions_json)
        _regions.push_back(WorldRegion::from_json(region_json, _nodes));

    std::cout << _regions.size() << " regions loaded." << std::endl;

    for(auto& [id, node] : _nodes)
        if(node->region() == nullptr)
            throw RandomizerException("Node '" + node->id() + "' doesn't belong to any region");
}

void World::init_spawn_locations()
{
    // Load base model
    Json spawns_json = Json::parse(SPAWN_LOCATIONS_JSON);
    for(auto& [id, spawn_json] : spawns_json.items())
        this->add_spawn_location(SpawnLocation::from_json(id, spawn_json, _nodes));
    std::cout << _spawn_locations.size() << " spawn locations loaded." << std::endl;

    // Patch model if user specified a model patch
    const Json& model_patch = _options.spawn_locations_model_patch();
    for(auto& [id, patch_json] : model_patch.items())
    {
        if(!_spawn_locations.count(id))
            this->add_spawn_location(SpawnLocation::from_json(id, patch_json, _nodes));
        else
            _spawn_locations.at(id)->apply_json(patch_json, _nodes);
    }
}

void World::init_hint_sources()
{
    Json hint_sources_json = Json::parse(HINT_SOURCES_JSON, nullptr, true, true);
    for(const Json& hint_source_json : hint_sources_json)
    {
        HintSource* new_source = HintSource::from_json(hint_source_json, _nodes, _game_strings);
        _hint_sources[new_source->description()] = new_source;
    }
    std::cout << _hint_sources.size() << " hint sources loaded." << std::endl;
}

void World::init_teleport_trees()
{
    Json trees_json = Json::parse(WORLD_TELEPORT_TREES_JSON);
    for(const Json& tree_pair_json : trees_json)
    {
        WorldTeleportTree* tree_1 = WorldTeleportTree::from_json(tree_pair_json[0], _nodes);
        WorldTeleportTree* tree_2 = WorldTeleportTree::from_json(tree_pair_json[1], _nodes);
        _teleport_tree_pairs.push_back(std::make_pair(tree_1, tree_2));
    }

    std::cout << _teleport_tree_pairs.size()  << " teleport tree pairs loaded." << std::endl;
}

void World::init_game_strings(const md::ROM& rom)
{
    TextbanksDecoder decoder(rom);
    _game_strings = decoder.strings();

    Json game_strings_json = Json::parse(GAME_STRINGS_JSON, nullptr, true, true);
    std::vector<std::string> strings_to_empty = game_strings_json.at("emptiedIndices");
    for(std::string string_hex_id : strings_to_empty)
    {
        uint16_t game_string_id = std::stoi(string_hex_id, 0, 16);
        _game_strings[game_string_id] = "";
    }

    for (auto& [string_hex_id, string_value] : game_strings_json.at("patches").items())
    {
        uint16_t game_string_id = std::stoi(string_hex_id, 0, 16);
        _game_strings[game_string_id] = string_value;
    }

    // Kazalt rejection message
    _game_strings[0x022] = std::string("Only the bearers of the ") 
        + std::to_string(_options.jewel_count()) + " jewels\n are worthy of entering\n King Nole's domain...\x1E";
}

void World::init_entity_types(const md::ROM& rom)
{
    // Read item drop probabilities from a table in the ROM
    std::vector<uint16_t> probability_table;
    for(uint32_t addr = offsets::PROBABILITY_TABLE ; addr < offsets::PROBABILITY_TABLE_END ; addr += 0x2)
        probability_table.push_back(rom.get_word(addr));

    // Read enemy info from a table in the ROM
    for(uint32_t addr = offsets::ENEMY_STATS_TABLE ; rom.get_word(addr) != 0xFFFF ; addr += 0x6)
    {
        uint8_t id = rom.get_byte(addr);
        std::string name = "enemy_" + std::to_string(id);
        uint8_t health = rom.get_byte(addr+1);
        uint8_t defence = rom.get_byte(addr+2);
        uint8_t dropped_golds = rom.get_byte(addr+3);
        uint8_t attack = rom.get_byte(addr+4) & 0x7F;
        Item* dropped_item = _items.at(rom.get_byte(addr+5) & 0x3F);

        // Use previously built probability table to know the real drop chances
        uint8_t probability_id = ((rom.get_byte(addr+4) & 0x80) >> 5) | (rom.get_byte(addr+5) >> 6);
        uint16_t drop_probability = probability_table.at(probability_id);

        _entity_types[id] = new EntityEnemy(id, name, health, attack, defence, dropped_golds, dropped_item, drop_probability);  
    }

    // Init ground item entity types
    for(auto& [item_id, item] : _items)
    {
        if(item_id > 0x3F)
            continue;
        uint8_t entity_id = item_id + 0xC0;
        _entity_types[entity_id] = new EntityItemOnGround(entity_id, "ground_item (" + item->name() + ")", item);
    }

    // Apply the randomizer model changes to the model loaded from ROM
    Json entities_json = Json::parse(ENTITIES_JSON);
    for(auto& [id_string, entity_json] : entities_json.items())
    {
        uint8_t id = std::stoi(id_string);
        if(!_entity_types.count(id))
            _entity_types[id] = EntityType::from_json(id, entity_json, *this);
        else
            _entity_types[id]->apply_json(entity_json, *this);
    }

    std::cout << _entity_types.size()  << " entities loaded." << std::endl;
}

WorldNode* World::spawn_node() const 
{ 
    return (_active_spawn_location) ? _active_spawn_location->node() : nullptr;
}

WorldNode* World::end_node() const 
{ 
    return _nodes.at("end");
} 

EntityType* World::entity_type(const std::string& name) const
{
    for(auto& [id, enemy] : _entity_types)
        if(enemy->name() == name)
            return enemy;
    return nullptr;
}

void World::init_maps(const md::ROM& rom)
{
    constexpr uint16_t MAP_COUNT = 816;
    for(uint16_t map_id = 0 ; map_id < MAP_COUNT ; ++map_id)
        _maps[map_id] = new Map(map_id, rom, *this);

    const std::set<uint16_t> UNREACHABLE_MAPS = {};
    for(uint16_t map_id : UNREACHABLE_MAPS)
        _maps.at(map_id)->clear();
}

void World::replace_map(uint16_t map_id, Map* map)
{
    if(!_maps.count(map_id))
        throw RandomizerException("Attempting to replace map #" + std::to_string(map_id) + " but it doesn't exist");

    delete _maps[map_id];
    map->id(map_id);
    _maps[map_id] = map;
}

void World::add_tree_logic_paths()
{
    if(_options.all_trees_visited_at_start())
    {
        std::vector<WorldNode*> required_nodes;
        if(!_options.remove_tibor_requirement())
            required_nodes = { _nodes["tibor"] };

        for(auto& pair : _teleport_tree_pairs)
        {
            this->add_path(new WorldPath(pair.first->node(), pair.second->node(), 1, {}, required_nodes));
            this->add_path(new WorldPath(pair.second->node(), pair.first->node(), 1, {}, required_nodes));
        }
    }
}

WorldRegion* World::region(const std::string& name) const
{
    for(WorldRegion* region : _regions)
        if(region->name() == name)
            return region;
    return nullptr;
}


Json World::to_json() const
{
    Json json;

    // Export dark node
    json["spawnLocation"] = _active_spawn_location->id();
    json["darkRegion"] = _dark_region->name();

    // Export hints
    for(auto& [description, source] : _hint_sources)
        json["hints"][description] = source->text();

    if(_options.shuffle_tibor_trees())
    {
        json["tiborTrees"] = Json::array();
        for(auto& pair : _teleport_tree_pairs)
            json["tiborTrees"].push_back(pair.first->name() + " <--> " + pair.second->name());
    }

    // Export item sources
    for(WorldRegion* region : _regions)
    {
        for(WorldNode* node : region->nodes())
        {
            for(ItemSource* source : node->item_sources())
            {
                Item* item = _items.at(source->item_id());
                json["itemSources"][region->name()][source->name()] = item->name();
            }
        }
    }

    // Fahl enemies
    json["fahlEnemies"] = Json::array();
    for(EntityType* enemy : _fahl_enemies)
        json["fahlEnemies"].push_back(enemy->name());

    return json;
}

void World::parse_json(const Json& json)
{
    ////////// Item Sources ///////////////////////////////////////////
    const Json& item_sources_json = json.at("itemSources");
    for(auto& it : _nodes)
    {
        const WorldNode& node = *it.second;
        if(node.item_sources().empty())
            continue;

        if(item_sources_json.contains(node.name()))
        {
            const Json& node_json = item_sources_json.at(node.name());
            for(ItemSource* source : node.item_sources())
            {
                if(node_json.contains(source->name()))
                {
                    std::string item_name = node_json.at(source->name());
                    Item* item = this->parse_item_from_name(item_name);
                    if(item)
                    {
                        source->item(item);
                    }
                    else
                    {
                        std::stringstream msg;
                        msg << "Item name '" << item_name << "' is invalid in plando JSON.";
                        throw JsonParsingException(msg.str());
                    }
                }
                else
                {
                    std::stringstream msg;
                    msg << "Item source '" << source->name() << "' is missing from plando JSON.";
                    throw JsonParsingException(msg.str());
                }
            }
        }
        else
        {
            std::stringstream msg;
            msg << "Node '" << node.name() << "' is missing from plando JSON.";
            throw JsonParsingException(msg.str());
        }
    }

    ////////// Hints ///////////////////////////////////////////
    const Json& hints_json = json.at("hints");
    for(auto& [description, source] : _hint_sources)
    {
        if(hints_json.contains(description))
        {
            if(hints_json.at(description).is_array())
            {
                std::vector<std::string> hint_lines;
                for(const std::string& line : hints_json.at(description))
                    hint_lines.push_back(line);
                std::string hint = tools::join(hint_lines, "\n");
                source->text(hint);
            }
            else
                source->text(hints_json.at(description));
        }
        else
        {
            std::stringstream msg;
            msg << "Hint source '" << description << "' is missing from plando JSON.";
            throw JsonParsingException(msg.str());
        }
    }

    ////////// Miscellaneous ///////////////////////////////////////////
    if(json.contains("spawnLocation"))
    {
        try {
            _active_spawn_location = _spawn_locations.at(json.at("spawnLocation"));
        } 
        catch(std::out_of_range&)
        {
            std::stringstream msg;
            msg << "Spawn location name '" << json.at("spawnLocation") << "' is invalid in plando JSON.";
            throw JsonParsingException(msg.str());
        }
    }

    // Parse dark node
    if(json.contains("darkRegion"))
    {
        std::string dark_region_name = json.at("darkRegion");
        _dark_region = this->region(dark_region_name);
        if(!_dark_region)
        {
            std::stringstream msg;
            msg << "Darkened region name '" << dark_region_name << "' is invalid in plando JSON.";
            throw JsonParsingException(msg.str());
        }
    }
    else throw JsonParsingException("Darkened region is missing from plando JSON.");

    // Parse Fahl enemies
    if(json.contains("fahlEnemies"))
    {
        for(std::string enemy_name : json.at("fahlEnemies"))
        {
            EntityType* enemy = this->entity_type(enemy_name);
            if(!enemy)
            {
                std::stringstream msg;
                msg << "Fahl enemy name '" << enemy_name << "' is invalid in plando JSON.";
                throw JsonParsingException(msg.str());
            }
            _fahl_enemies.push_back(enemy);
        }            
    }
}

Item* World::parse_item_from_name(const std::string& item_name)
{
    Item* item = this->item(item_name);
    if(item)
        return item;

    // If item is formatted as "X golds", parse X value and create the matching gold stack item
    if(tools::ends_with(item_name, "golds"))
    {
        size_t space_index = item_name.find_first_of(' ');
        if(space_index == std::string::npos)
            return nullptr;

        std::string number_part = item_name.substr(0, space_index);
        
        try {
            uint8_t gold_value = static_cast<uint8_t>(std::strtol(number_part.c_str(), nullptr, 10));
            return this->add_gold_item(gold_value);
        }
        catch(std::exception&) {}
    }

    return nullptr;
}

bool World::is_region_avoidable(WorldRegion* region) const
{
    WorldSolver solver(*this);
    solver.forbid_taking_items_from_nodes(region->nodes());
    return solver.try_to_solve();
}

bool World::is_item_avoidable(Item* item) const
{
    WorldSolver solver(*this);
    solver.forbid_item_types({ item });
    return solver.try_to_solve();
}

void World::output_model()
{
    Json hints_json = Json::array();
    for(auto& [id, hint_source] : _hint_sources)
        hints_json.push_back(hint_source->to_json());
    tools::dump_json_to_file(hints_json, "./json_data/hint_source.json");

    Json item_sources_json = Json::array();
    for(ItemSource* source : _item_sources)
        item_sources_json.push_back(source->to_json());
    tools::dump_json_to_file(item_sources_json, "./json_data/item_source.json");

    Json items_json;
    for(auto& [id, item] : _items)
        items_json[std::to_string(id)] = item->to_json();
    tools::dump_json_to_file(items_json, "./json_data/item.json");

    Json regions_json = Json::array();
    for(WorldRegion* region : _regions)
        regions_json.push_back(region->to_json());
    tools::dump_json_to_file(regions_json, "./json_data/world_region.json");

    Json spawns_json;
    for(auto& [id, spawn] : _spawn_locations)
        spawns_json[id] = spawn->to_json();
    tools::dump_json_to_file(spawns_json, "./json_data/world_spawns.json");

    Json nodes_json;
    for(auto& [id, node] : _nodes)
        nodes_json[id] = node->to_json();
    tools::dump_json_to_file(nodes_json, "./json_data/world_node.json");

    Json trees_json = Json::array();
    for(auto& [tree_1, tree_2] : _teleport_tree_pairs)
    {
        Json pair_json = Json::array();
        pair_json.push_back(tree_1->to_json());
        pair_json.push_back(tree_2->to_json());
        trees_json.push_back(pair_json);
    }
    tools::dump_json_to_file(trees_json, "./json_data/world_teleport_tree.json");

    Json strings_json;
    for(uint32_t i=0 ; i<_game_strings.size() ; ++i)
    {
        std::stringstream hex_id;
        hex_id << "0x" << std::hex << i;
        strings_json[hex_id.str()] = _game_strings[i];
    }
    tools::dump_json_to_file(strings_json, "./json_data/game_strings.json");

    auto paths_copy = _paths;
    Json paths_json = Json::array();
    while(!paths_copy.empty())
    {
        auto it = paths_copy.begin();
        std::pair<WorldNode*, WorldNode*> node_pair = it->first;
        WorldPath* path = it->second;
        paths_copy.erase(it);

        bool two_way = false;
        std::pair<WorldNode*, WorldNode*> reverse_pair = std::make_pair(node_pair.second, node_pair.first);
        if(paths_copy.count(reverse_pair))
        {
            WorldPath* reverse_path = paths_copy.at(reverse_pair);
            if(path->is_perfect_opposite_of(reverse_path))
            {
                two_way = true;
                paths_copy.erase(reverse_pair);
            }
        }
        
        paths_json.push_back(path->to_json(two_way));
    }
    tools::dump_json_to_file(paths_json, "./json_data/world_path.json");

    Json entity_types_json = Json::object();
    for(auto& [id, entity_type] : _entity_types)
        entity_types_json[std::to_string(id)] = entity_type->to_json();
    tools::dump_json_to_file(entity_types_json, "./json_data/entity_type.json");

    Json maps_json = Json::object();
    for(auto& [map_id, map] : _maps)
        maps_json[std::to_string(map_id)] = map->to_json(*this);
    tools::dump_json_to_file(maps_json, "./json_data/map.json");
}
