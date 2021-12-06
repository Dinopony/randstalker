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
        free_map_ids.emplace_back(map->id());
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

void create_chest_item_source(World& world, uint8_t chest_id, uint8_t item_id)
{
    ItemSource* source = new ItemSourceChest(chest_id, "");
    source->item(world.item(item_id));
    world.item_sources().emplace_back(source);
}

void apply_kaizo_edits(World& world, md::ROM& rom)
{
    world.starting_golds(100);
    world.item(ITEM_GOLA_EYE)->starting_quantity(1);
    world.item(ITEM_EINSTEIN_WHISTLE)->starting_quantity(1);

    for (auto& [id, entity_type] : world.entity_types())
    {
        if(entity_type->type_name() != "enemy")
            continue;
        EntityEnemy* enemy_type = reinterpret_cast<EntityEnemy*>(entity_type);

        enemy_type->apply_damage_factor(1.25);
        enemy_type->apply_health_factor(1.50);
        enemy_type->apply_golds_factor(1.25);
        enemy_type->apply_drop_chance_factor(1.50);
    }

    patch_engine_for_kaizo(world, rom);
    edit_safety_pass_arc(world, rom);
    edit_mercator(world, rom);
    edit_mercator_castle(world, rom);

    std::cout << "Free map IDs:\n";
    for(uint16_t map_id : free_map_ids)
        std::cout << "- " << map_id << "\n";
}
