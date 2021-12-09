#pragma once

#include <landstalker_lib/model/world.hpp>
#include "../logic_model/world_logic.hpp"

class WorldJsonParser
{
public:
    static void parse_world_json(World& world, WorldLogic& logic, const Json& json);

private:
    WorldJsonParser() = default;
};

