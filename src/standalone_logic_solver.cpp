#include <landstalker-lib/tools/argument_dictionary.hpp>
#include <iostream>

#include "logic_model/randomizer_world.hpp"
#include "logic_model/item_source.hpp"
#include "randomizer_options.hpp"
#include "world_solver.hpp"
#include "apply_randomizer_options.hpp"
#include "io/io.hpp"
#include "world_shuffler.hpp"

int solve_logic(ArgumentDictionary& args)
{
    args.set_boolean("silent", true);

    RandomizerWorld world;

    // Do a quick and dirty setup of things that would usually get loaded from ROM for faster execution times
    for(uint8_t id=0 ; id < 64 ; ++id)
        world.add_item(new Item(id, "", 1, 0, 0, 0));
    world.chest_contents().resize(222, world.item(ITEM_NONE));
    world.game_strings().resize(2175);
    world.load_model_from_json(true);

    // Parse options to get info such as the starting region, the goal or the current inventory (expressed as starting inventory)
    RandomizerOptions options(args, world.item_names(), world.spawn_location_names());
    apply_randomizer_options(options, world);

    // Parse a potential "world" section inside the preset for plandos & half plandos
    WorldJsonParser::parse_world_json(world, options.world_json());
    for(ItemSource* source : world.item_sources())
        source->item(world.item(ITEM_NONE));

    // Perform a light version of randomization, just to randomize meta elements (spawn location, dark dungeon, trees...)
    WorldShuffler shuffler(world, options);
    shuffler.pre_randomize_for_logic_solver();

    // Solve the actual logic
    WorldSolver solver(world);
    solver.setup(world.spawn_node(), world.end_node(), world.starting_inventory());
    solver.run_until_blocked();

    Json output_json = Json::array();
    for(ItemSource* item_source : solver.reachable_item_sources())
        output_json.emplace_back(item_source->name());

    std::ofstream file("./reachable_sources.json");
    file << output_json.dump(4);
    file.close();

    return EXIT_SUCCESS;
}