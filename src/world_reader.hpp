#pragma once

#include "tools/megadrive/rom.hpp"
class World;

class WorldReader {
public:
    static void read_maps(World& world, const md::ROM& rom);
    static void read_map_connections(World& world, const md::ROM& rom);
    static void read_map_palettes(World& world, const md::ROM& rom);

private:
    static void read_maps_data(World& world, const md::ROM& rom);
    static void read_maps_fall_destination(World& world, const md::ROM& rom);
    static void read_maps_climb_destination(World& world, const md::ROM& rom);
    static void read_maps_entities(World& world, const md::ROM& rom);
    static void read_maps_variants(World& world, const md::ROM& rom);
    static void read_maps_entity_masks(World& world, const md::ROM& rom);
    static void read_maps_global_entity_masks(World& world, const md::ROM& rom);
    static void read_maps_dialogue_table(World& world, const md::ROM& rom);
    static void read_persistence_flags(World& world, const md::ROM& rom);

    WorldReader() {}
};
