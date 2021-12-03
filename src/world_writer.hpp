#pragma once

#include "tools/megadrive/rom.hpp"
class World;

class WorldWriter {
public:
    static void write_world_to_rom(md::ROM& rom, const World& world);

private:
    static void write_items(md::ROM& rom, const World& world);
    static void write_item_sources(md::ROM& rom, const World& world);
    static void write_entity_types(md::ROM& rom, const World& world);
    static void write_game_strings(md::ROM& rom, const World& world);
    static void write_dark_rooms(md::ROM& rom, const World& world);
    static void write_tibor_tree_connections(md::ROM& rom, const World& world);
    static void write_fahl_enemies(md::ROM& rom, const World& world);

    static void write_maps(md::ROM& rom, const World& world);
    static void write_maps_data(md::ROM& rom, const World& world);
    static void write_maps_fall_destination(md::ROM& rom, const World& world);
    static void write_maps_climb_destination(md::ROM& rom, const World& world);
    static void write_maps_variants(md::ROM& rom, const World& world);
    static void write_maps_global_entity_masks(md::ROM& rom, const World& world);
    static void write_maps_entity_masks(md::ROM& rom, const World& world);
    static void write_maps_dialogue_table(md::ROM& rom, const World& world);
    static void write_maps_entities(md::ROM& rom, const World& world);

    WorldWriter() {}
};
