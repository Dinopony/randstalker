#pragma once

#include <string>

class World;


namespace graphviz {

void output_logic_as_dot(const World& world, const std::string& path);
void output_maps_as_dot(const World& world, const std::string& path);

} // namespace end

