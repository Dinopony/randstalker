#pragma once

#include <landstalker-lib/tools/json.hpp>
#include <string>

class RandomizerWorld;
class RandomizerOptions;

namespace ModelWriter {
    void write_world_model(const RandomizerWorld& world);
    void write_logic_model(const RandomizerWorld& world);
}

namespace GraphvizWriter {
    void write_logic_as_dot(const RandomizerWorld& world, const std::string& path);
    void write_maps_as_dot(const RandomizerWorld& world, const std::string& path);
}

namespace WorldJsonParser {
    void parse_world_json(RandomizerWorld& world, const Json& json);
}

namespace WorldJsonWriter {
    Json build_world_json(const RandomizerWorld& world, const RandomizerOptions& options);
}
