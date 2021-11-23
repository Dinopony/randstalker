//////////////////////////////////////////////////////////////////////////////////////////
//
//     RANDSTALKER ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//     Developed by: Dinopony (@DinoponyRuns)
//
// ---------------------------------------------------------------------------------------
//
//  Thanks to the whole Landstalker speedrunning community for being supportive during the whole process of developing this
//  Special mention to Wizardwhosaysni for being extra helpful with his deep knowledge of Megadrive reverse-engineering
// 
//////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <iostream>

#include "extlibs/base64.hpp"

#include "model/world_node.hpp"
#include "model/item_source.hpp"

#include "patches/patches.hpp"

#include "tools/megadrive/rom.hpp"
#include "tools/argument_dictionary.hpp"
#include "tools/tools.hpp"

#include "exceptions.hpp"
#include "offsets.hpp"
#include "world.hpp"
#include "world_writer.hpp"
#include "world_randomizer.hpp"

md::ROM* get_input_rom(std::string input_rom_path)
{
    md::ROM* rom = new md::ROM(input_rom_path);
    while (!rom->is_valid())
    {
        delete rom;
        if (!input_rom_path.empty())
            std::cout << "[ERROR] ROM input path \"" << input_rom_path << "\" is wrong, and no ROM could be opened this way.\n\n";
        std::cout << "Please specify input ROM path (or drag ROM on Randstalker.exe icon before launching): ";
        std::getline(std::cin, input_rom_path);
        rom = new md::ROM(input_rom_path);
    }

    return rom;
}

void process_paths(const ArgumentDictionary& args, const RandomizerOptions& options,
                    std::string& input_rom_path, std::string& output_rom_path, std::string& spoiler_log_path)
{
    input_rom_path = args.get_string("inputrom", "./input.md");
    output_rom_path = args.get_string("outputrom", "./");
    spoiler_log_path = args.get_string("outputlog", "./");

    // Clean output ROM path and determine if it's a directory or a file
    bool output_path_is_a_directory = true;
    if(!output_rom_path.empty())
    {
        output_path_is_a_directory = !tools::ends_with(output_rom_path, ".md") && !tools::ends_with(output_rom_path, ".bin");
        if(output_path_is_a_directory && *output_rom_path.rbegin() != '/')
            output_rom_path += "/";
    }

    // Clean output log path and if it wasn't specified, give it an appropriate default value
    if(!args.contains("outputlog"))
    {
        if(output_path_is_a_directory && !output_rom_path.empty())
            spoiler_log_path = output_rom_path; // outputRomPath points to a directory, use the same for the spoiler log
        else
            spoiler_log_path = "./"; // outputRomPath points to a file, use cwd for the spoiler log
    }
    else if(!spoiler_log_path.empty() && !tools::ends_with(spoiler_log_path, ".json") && *spoiler_log_path.rbegin() != '/')
        spoiler_log_path += "/";

    // Add the filename afterwards
    if(!output_rom_path.empty() && *output_rom_path.rbegin() == '/')
        output_rom_path += options.hash_sentence() + ".md";
    if(!spoiler_log_path.empty() && *spoiler_log_path.rbegin() == '/')
        spoiler_log_path += options.hash_sentence() + ".json";
}

Json randomize(World& world, RandomizerOptions& options, const ArgumentDictionary& args)
{
    Json spoiler_json;

    std::string permalink = options.permalink();
    spoiler_json["permalink"] = permalink;
    spoiler_json["hashSentence"] = options.hash_sentence();
    spoiler_json.merge_patch(options.to_json());

    std::cout << "Permalink: " << permalink << "\n\n";
    std::cout << "Share the permalink above with other people to enable them building the exact same seed.\n" << std::endl;

    // In rando mode, we rock our little World and shuffle things around to make a brand new experience on each seed.
    std::cout << "Randomizing world...\n";
    WorldRandomizer randomizer(world, options);
    randomizer.randomize();

    std::string debug_log_path = args.get_string("debuglog");
    if (!debug_log_path.empty())
    {
        std::ofstream debug_log_file(debug_log_path);
        debug_log_file << randomizer.debug_log_as_json().dump(4);
        debug_log_file.close();
    }

    spoiler_json.merge_patch(world.to_json());
    spoiler_json["playthrough"] = randomizer.playthrough_as_json();

    return spoiler_json;
}

Json plandomize(World& world, RandomizerOptions& options, const ArgumentDictionary& args)
{
    std::cout << "Plandomizing world...\n";
    Json spoiler_json;

    // In plando mode, we parse the world from the file given as a plando input, without really randomizing anything.
    // The software will act as a simple ROM patcher, without verifying the game is actually completable.

    world.parse_json(options.input_plando_json());

    spoiler_json.merge_patch(options.to_json());
    spoiler_json.merge_patch(world.to_json());

    // If --encodePlando is passed, the plando being processed is outputted in an encoded fashion
    if (args.get_boolean("encodeplando") && options.is_plando())
    {
        std::ofstream encodedPlandoFile("./encoded_plando.json");
        Json fileJson;
        fileJson["plando_permalink"] = base64_encode(Json::to_msgpack(spoiler_json));
        encodedPlandoFile << fileJson.dump(4);
        encodedPlandoFile.close();

        std::cout << "Plando encoded to './encoded_plando.json'" << std::endl;
        exit(0);
    }

    return spoiler_json;
}

void display_options(const RandomizerOptions& options)
{
    Json options_as_json = options.to_json();
    if(!options.has_custom_mandatory_items())
        options_as_json["randomizerSettings"]["mandatoryItems"] = "default";
    if(!options.has_custom_filler_items())
        options_as_json["randomizerSettings"]["fillerItems"] = "default";
    std::cout << "Settings: " << options_as_json.dump(2) << "\n\n";
}

void generate(const ArgumentDictionary& args)
{
    // Parse options from command-line args, preset file, plando file...
    RandomizerOptions options(args);

    // Parse various paths from args
    std::string input_rom_path, output_rom_path, spoiler_log_path;
    process_paths(args, options, input_rom_path, output_rom_path, spoiler_log_path);
 
    // Output current preset
    if (args.get_boolean("writepreset"))
    {
        Json json = options.to_json();

        std::ofstream presetFile("./preset.json");
        if(presetFile)
            presetFile << json.dump(4);
        presetFile.close();

        std::cout << "Preset written to './preset.json'" << std::endl;
        return;
    }

    // Load input ROM and tag known empty chunks of data to know where to inject code / data
    md::ROM* rom = get_input_rom(input_rom_path);
    rom->mark_empty_chunk(offsets::LITHOGRAPH_TILES, offsets::LITHOGRAPH_TILES_END);
    rom->mark_empty_chunk(0x11F380, 0x120000); // Empty space
    rom->mark_empty_chunk(0x1FFAC0, 0x200000); // Empty space

    World world(*rom, options);
    apply_world_edits(world, options, *rom);

    display_options(options);

    Json spoiler_json;
    if(options.is_plando())
    {
        spoiler_json = plandomize(world, options, args);
    }
    else
    {
        spoiler_json = randomize(world, options, args);
    }
    
    // Output world to ROM and save ROM unless it was explicitly specified by the user not to output a ROM
    if(!output_rom_path.empty())
    {
        std::cout << "Writing world to ROM...\n";
        WorldWriter::write_world_to_rom(*rom, world);

        // Apply patches to the game ROM to alter various things that are not directly part of the game world randomization
        std::cout << "Applying game patches...\n\n";
        apply_game_patches(*rom, options, world);

        std::ofstream output_rom_file(output_rom_path, std::ios::binary);
        if(!output_rom_file)
            throw RandomizerException("Could not open output ROM file for writing at path '" + output_rom_path + "'");

        rom->write_to_file(output_rom_file);
        std::cout << "Randomized rom outputted to \"" << output_rom_path << "\".\n\n";
    }

    // Output current world model if requested
    if (args.get_boolean("dumpmodel"))
    {
        if(options.allow_spoiler_log())
        {
            world.output_model();
            std::cout << "Model dumped to './json_data/'" << std::endl;
        }
        else
            std::cout << "Dumping model is not authorized on seeds with spoiler log disabled, it won't be generated.\n\n";
    }

    // Write a spoiler log to help the player
    if(!spoiler_log_path.empty())
    {
        if(options.allow_spoiler_log())
        {
            std::ofstream spoiler_file(spoiler_log_path);
            if(!spoiler_file)
                throw RandomizerException("Could not open output log file for writing at path '" + spoiler_log_path + "'");

            spoiler_file << spoiler_json.dump(4);
            spoiler_file.close();
            std::cout << "Spoiler log written into \"" << spoiler_log_path << "\".\n\n";
        }
        else
            std::cout << "Spoiler log is not authorized under these settings, it won't be generated.\n\n";
    }
}

int main(int argc, char* argv[])
{
    int return_code = EXIT_SUCCESS;

    ArgumentDictionary args(argc, argv);

    std::cout << "======== Randstalker v" << RELEASE << " ========\n\n";

    try
    {
        int seed_count = args.get_integer("seedcount", 1);
        for(int i=0 ; i<seed_count ; ++i)
            generate(args);
    }
    catch(RandomizerException& e) 
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return_code = EXIT_FAILURE;
    }

    if(args.get_boolean("pause", true))
    {
        std::cout << "Press any key to exit.";
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    return return_code;
}
