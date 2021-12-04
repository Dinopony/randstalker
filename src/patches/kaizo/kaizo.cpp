#include "./kaizo.hpp"

#include <iostream>

static std::vector<uint16_t> free_map_ids;

void batch_add_entities(Map* map, const std::vector<Position>& positions, Entity::Attributes base_attributes,  bool use_tiles_from_first)
{
    Entity* first = nullptr;
    for(auto& position : positions)
    {
        Entity::Attributes attributes = base_attributes;
        attributes.position = position;
        attributes.entity_to_use_tiles_from = (first && use_tiles_from_first) ? (first) : nullptr;

        Entity* entity = map->add_entity(new Entity(attributes));
        if(!first)
            first = entity;
    }
}

void wipe_map_variants(Map* map)
{
    for(auto& [variant_map, flag] : map->variants())
    {
        free_map_ids.push_back(map->id());
        variant_map->clear();
    }
    map->variants().clear();
}

void wipe_map_variants(World& world, const std::vector<uint16_t>& map_ids)
{
    for(uint16_t map_id : map_ids)
        wipe_map_variants(world.map(map_id));
}

void make_entity_visible_with_golas_eye(Entity* entity)
{
    entity->behavior_id(BEHAVIOR_INVISIBLE_REVEALABLE_BY_GOLAS_EYE);
}

void apply_kaizo_edits(World& world, md::ROM& rom)
{
    patch_engine_for_kaizo(world, rom);
    edit_safety_pass_arc(world, rom);
    edit_mercator(world, rom);
    edit_mercator_castle(world, rom);

    std::cout << "Free map IDs:\n";
    for(uint16_t map_id : free_map_ids)
        std::cout << "- " << map_id << "\n";
}
