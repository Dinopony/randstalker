#include "./kaizo.hpp"

void apply_kaizo_edits(World& world, md::ROM& rom)
{
    patch_engine_for_kaizo(world, rom);
    edit_safety_pass_arc(world, rom);
    edit_mercator(world, rom);
    edit_mercator_castle(world, rom);
}
