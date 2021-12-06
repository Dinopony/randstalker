#include "world.hpp"

#include <iostream>

#include "../tools/textbanks_decoder.hpp"
#include "../tools/textbanks_encoder.hpp"

#include "entity_type.hpp"
#include "map.hpp"
#include "item.hpp"
#include "item_source.hpp"
#include "map.hpp"
#include "map_palette.hpp"
#include "spawn_location.hpp"
#include "world_teleport_tree.hpp"

#include "../exceptions.hpp"
#include "../constants/offsets.hpp"
#include "../world_reader.hpp"
#include "../world_writer.hpp"

// Include headers automatically generated from model json files
#include "data/entity_type.json.hxx"
#include "data/item.json.hxx"
#include "data/item_source.json.hxx"
#include "data/world_teleport_tree.json.hxx"
#include "../assets/game_strings.json.hxx"

World::World(const md::ROM& rom)
{
    // No requirements
    this->init_items();
    this->init_game_strings(rom);
    this->init_entity_types(rom);

    // Reading map entities might actually require items
    WorldReader::read_map_palettes(*this, rom);
    WorldReader::read_maps(*this, rom);
    WorldReader::read_map_connections(*this, rom);

    // Require nodes, maps & entities
    this->init_item_sources();
    this->init_teleport_trees();
}

World::~World()
{
    for (auto& [key, item] : _items)
        delete item;
    for (ItemSource* source : _item_sources)
        delete source;
    for (auto& [tree_1, tree_2] : _teleport_tree_pairs)
    {
        delete tree_1;
        delete tree_2;
    }
    for (auto& [id, entity] : _entity_types)
        delete entity;
    for (MapPalette* palette : _map_palettes)
        delete palette;
}

void World::write_to_rom(md::ROM& rom)
{
    this->clean_unused_palettes();
    WorldWriter::write_world_to_rom(rom, *this);
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
            starting_inventory.emplace_back(item);
    }
    return starting_inventory;
}

std::vector<ItemSource*> World::item_sources_with_item(Item* item)
{
    std::vector<ItemSource*> sources_with_item;

    for (ItemSource* source : _item_sources)
        if (source->item() == item)
            sources_with_item.emplace_back(source);

    return sources_with_item;
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
}

void World::init_item_sources()
{
    Json item_sources_json = Json::parse(ITEM_SOURCES_JSON);
    for(const Json& source_json : item_sources_json)
    {
        _item_sources.emplace_back(ItemSource::from_json(source_json, *this));
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

void World::init_teleport_trees()
{
    Json trees_json = Json::parse(WORLD_TELEPORT_TREES_JSON);
    for(const Json& tree_pair_json : trees_json)
    {
        WorldTeleportTree* tree_1 = WorldTeleportTree::from_json(tree_pair_json[0]);
        WorldTeleportTree* tree_2 = WorldTeleportTree::from_json(tree_pair_json[1]);
        _teleport_tree_pairs.emplace_back(std::make_pair(tree_1, tree_2));
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
}

void World::init_entity_types(const md::ROM& rom)
{
    // Read item drop probabilities from a table in the ROM
    std::vector<uint16_t> probability_table;
    for(uint32_t addr = offsets::PROBABILITY_TABLE ; addr < offsets::PROBABILITY_TABLE_END ; addr += 0x2)
        probability_table.emplace_back(rom.get_word(addr));

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

uint8_t World::starting_life() const
{
    if(_custom_starting_life)
        return _custom_starting_life;
    return _spawn_location.starting_life();
}

EntityType* World::entity_type(const std::string& name) const
{
    for(auto& [id, enemy] : _entity_types)
        if(enemy->name() == name)
            return enemy;
    return nullptr;
}

void World::set_map(uint16_t map_id, Map* map)
{
    if(_maps.count(map_id))
        delete _maps[map_id];
 
    map->id(map_id);
    _maps[map_id] = map;
}



MapConnection& World::map_connection(uint16_t map_id_1, uint16_t map_id_2)
{
    for(MapConnection& connection : _map_connections)
        if(connection.check_maps(map_id_1, map_id_2))
            return connection;

    throw RandomizerException("Could not find a map connection between maps " + std::to_string(map_id_1) + " and " + std::to_string(map_id_2));
}

void World::swap_map_connections(uint16_t map_id_1, uint16_t map_id_2, uint16_t map_id_3, uint16_t map_id_4)
{
    MapConnection& conn_1 = map_connection(map_id_1, map_id_2);
    MapConnection& conn_2 = map_connection(map_id_3, map_id_4);

    bool right_order = true;
    if(conn_1.map_id_1() != map_id_1)
        right_order = !right_order;
    if(conn_2.map_id_1() != map_id_3)
        right_order = !right_order;
    
    uint16_t stored_map_id = conn_1.map_id_1();
    uint8_t stored_pos_x = conn_1.pos_x_1();
    uint8_t stored_pos_y = conn_1.pos_y_1();
    uint8_t stored_extra_byte = conn_1.extra_byte_1();

    if(right_order)
    {
        conn_1.map_id_1(conn_2.map_id_1());
        conn_1.pos_x_1(conn_2.pos_x_1()); 
        conn_1.pos_y_1(conn_2.pos_y_1());
        conn_1.extra_byte_1(conn_2.extra_byte_1());

        conn_2.map_id_1(stored_map_id);
        conn_2.pos_x_1(stored_pos_x); 
        conn_2.pos_y_1(stored_pos_y);
        conn_2.extra_byte_1(stored_extra_byte);
    }
    else
    {
        conn_1.map_id_1(conn_2.map_id_2());
        conn_1.pos_x_1(conn_2.pos_x_2()); 
        conn_1.pos_y_1(conn_2.pos_y_2());
        conn_1.extra_byte_1(conn_2.extra_byte_2());

        conn_2.map_id_2(stored_map_id);
        conn_2.pos_x_2(stored_pos_x); 
        conn_2.pos_y_2(stored_pos_y);
        conn_2.extra_byte_2(stored_extra_byte);
    }
}

uint8_t World::map_palette_id(MapPalette* palette) const
{
    for(uint8_t i=0 ; i<_map_palettes.size() ; ++i)
        if(_map_palettes[i] == palette)
            return i;

    throw RandomizerException("Could not find id of MapPalette as it doesn't seem to be in world's map palette list");
}

void World::clean_unused_palettes()
{
   std::set<MapPalette*> used_palettes;
    for(auto& [map_id, map] : _maps)
        used_palettes.insert(map->palette());

    for(auto it = _map_palettes.begin() ; it != _map_palettes.end() ; )
    {
        MapPalette* palette = *it;
        if(!used_palettes.count(palette))
        {
            delete palette;
            it = _map_palettes.erase(it);
        }
        else ++it;
    }
}

/*
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
                    hint_lines.emplace_back(line);
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
            _spawn_location = _spawn_locations.at(json.at("spawnLocation"));
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
            _fahl_enemies.emplace_back(enemy);
        }            
    }
}
*/

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
