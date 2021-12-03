#include "./kaizo.hpp"

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

void apply_kaizo_edits(World& world, md::ROM& rom)
{
    patch_engine_for_kaizo(world, rom);
    edit_safety_pass_arc(world, rom);
    edit_mercator(world, rom);
    edit_mercator_castle(world, rom);
}
