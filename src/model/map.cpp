#include "map.hpp"
#include "entity.hpp"

#include "../exceptions.hpp"
#include "../world.hpp"
#include "../offsets.hpp"

Map::Map(uint16_t map_id) :
    _id                 (map_id),
    _fall_destination   (0xFFFF),
    _climb_destination  (0xFFFF),
    _parent_map         (nullptr)
{}

Map::Map(const Map& map) :
    _id                         (map._id),
    _address                    (map._address),
    _tileset_id                 (map._tileset_id),
    _primary_big_tileset_id     (map._primary_big_tileset_id),
    _secondary_big_tileset_id   (map._secondary_big_tileset_id),
    _palette_id                 (map._palette_id),
    _room_height                (map._room_height),
    _background_music           (map._background_music),
    _unknown_param_1            (map._unknown_param_1),
    _unknown_param_2            (map._unknown_param_2),
    _base_chest_id              (map._base_chest_id),
    _fall_destination           (map._fall_destination),
    _climb_destination          (map._climb_destination),
    _parent_map                 (map._parent_map),
    _exits                      (map._exits),
    _variants                   (map._variants),
    _global_entity_mask_flags   (map._global_entity_mask_flags)
{
    for(Entity* entity : map._entities)
        this->add_entity(new Entity(*entity));
}

Map::~Map()
{
    for(Entity* entity : _entities)
        delete entity;
}

void Map::clear()
{
    _base_chest_id = 0x00;
    _fall_destination = 0xFFFF;
    _climb_destination = 0xFFFF;
    _entities.clear();
    _exits.clear();
    _variants.clear();
    _global_entity_mask_flags.clear();
    _speaker_ids.clear();
}

////////////////////////////////////////////////////////////////

Entity* Map::add_entity(Entity* entity) 
{
    entity->map(this);
    _entities.push_back(entity);
    return entity;
}

void Map::insert_entity(uint8_t entity_id, Entity* entity) 
{
    entity->map(this);
    _entities.insert(_entities.begin() + entity_id, entity);

    // Shift entity indexes in clear flags
    for(uint8_t i=0 ; i<_global_entity_mask_flags.size() ; ++i)
    {
        GlobalEntityMaskFlag& global_mask_flag = _global_entity_mask_flags[i];
        if(entity_id <= global_mask_flag.first_entity_id)
            global_mask_flag.first_entity_id++;
    }
}

uint8_t Map::entity_id(const Entity* entity) const
{
    for(uint8_t id=0 ; id < _entities.size() ; ++id)
        if(_entities[id] == entity)
            return id;
    throw RandomizerException("Prompting entity ID of entity not inside map.");
}

void Map::remove_entity(uint8_t entity_id, bool delete_pointer) 
{ 
    Entity* erased_entity = _entities.at(entity_id);
    _entities.erase(_entities.begin() + entity_id);

    // If any other entity were using tiles from this entity, clear that
    for(Entity* entity : _entities)
    {
        if(entity->entity_to_use_tiles_from() == erased_entity)
            entity->entity_to_use_tiles_from(nullptr);
    }

    // Shift entity indexes in clear flags
    for(uint8_t i=0 ; i<_global_entity_mask_flags.size() ; ++i)
    {
        GlobalEntityMaskFlag& global_mask_flag = _global_entity_mask_flags[i];
        if(global_mask_flag.first_entity_id >= _entities.size())
        {
            _global_entity_mask_flags.erase(_global_entity_mask_flags.begin() + i);
            --i;
        }
        else if(global_mask_flag.first_entity_id > entity_id)
            global_mask_flag.first_entity_id--;
    }

    if(delete_pointer)
        delete erased_entity;
}

void Map::move_entity(uint8_t entity_id, uint8_t entity_new_id)
{
    Entity* entity_to_move = _entities.at(entity_id); 
    this->remove_entity(entity_id, false);
    this->insert_entity(entity_new_id, entity_to_move);
} 

void Map::clear_entities()
{
    _entities.clear();
}

////////////////////////////////////////////////////////////////

void Map::add_variant(Map* variant_map, uint8_t flag_byte, uint8_t flag_bit)
{
    _variants[variant_map] = Flag(flag_byte, flag_bit);
    variant_map->_parent_map = this;
}

////////////////////////////////////////////////////////////////

Json Map::to_json(const World& world) const
{
    Json json;

    json["address"] = _address;
    json["tilesetId"] = _tileset_id;
    json["primaryBigTilesetId"] = _primary_big_tileset_id;
    json["secondaryBigTilesetId"] = _secondary_big_tileset_id;

    json["paletteId"] = _palette_id;
    json["roomHeight"] = _room_height;
    json["backgroundMusic"] = _background_music;
    json["baseChestId"] = _base_chest_id;

    if(_fall_destination != 0xFFFF)
        json["fallDestination"] = _fall_destination;
    if(_climb_destination != 0xFFFF)
        json["climbDestination"] = _climb_destination;

    if(!_exits.empty())
    {
        json["exits"] = Json::array();
        for(const MapExit& exit : _exits)
            json["exits"].push_back(exit.to_json());
    }

    json["visitedFlag"] = _visited_flag.to_json();

    if(!_variants.empty())
    {
        json["variants"] = Json::array();
        for(auto& [map, flag] : _variants)
        {
            json["variants"].push_back({
                { "mapVariantId", map->id() },
                { "flagByte", flag.byte },
                { "flagBit", flag.bit }
            });
        }
    }

    if(!_global_entity_mask_flags.empty())
    {
        json["globalEntityMaskFlags"] = Json::array();
        for(const GlobalEntityMaskFlag& global_mask_flag : _global_entity_mask_flags)
            json["globalEntityMaskFlags"].push_back(global_mask_flag.to_json());
    }

    if(!_entities.empty())
    {
        json["entities"] = Json::array();
        uint8_t chest_id = _base_chest_id;
        for(Entity* entity : _entities)
        {
            Json entity_json = entity->to_json(world);
            if(entity_json.at("entityType") == "chest")
                entity_json["chestId"] = chest_id++;
            json["entities"].push_back(entity_json);
        }
    }
    
    if(!_speaker_ids.empty())
        json["speakers"] = _speaker_ids;

    json["unknownParam1"] = _unknown_param_1;
    json["unknownParam2"] = _unknown_param_2;

    return json;
}
