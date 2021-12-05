#pragma once

#include "../extlibs/json.hpp"
#include <string>

class World;
class WorldLogic;
class WorldRandomizer;
class RandomizerOptions;

namespace ModelWriter {
    void write_world_model(const World& world);
    void write_logic_model(const WorldLogic& logic);
    void write_randomizer_model(const WorldRandomizer& randomizer);
}

namespace SpoilerWriter {
    Json build_spoiler_json(const World& world, const WorldRandomizer& randomizer, const WorldLogic& logic, const RandomizerOptions& options);
}

namespace GraphvizWriter {
    void write_logic_as_dot(const World& world, const std::string& path);
    void write_maps_as_dot(const World& world, const std::string& path);
}