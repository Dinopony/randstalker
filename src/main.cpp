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

#include <string>
#include <iostream>

#include <landstalker_lib/tools/json.hpp>
#include <landstalker_lib/md_tools.hpp>
#include <landstalker_lib/tools/argument_dictionary.hpp>
#include <landstalker_lib/constants/offsets.hpp>
#include <landstalker_lib/exceptions.hpp>

#include "tools/base64.hpp"
#include "patches/patches.hpp"
#include "apply_randomizer_options.hpp"
#include "logic_model/hint_source.hpp"
#include "logic_model/randomizer_world.hpp"
#include "world_shuffler.hpp"
#include "io/io.hpp"

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
        output_path_is_a_directory = !output_rom_path.ends_with(".md") && !output_rom_path.ends_with(".bin");
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
    else if(!spoiler_log_path.empty() && !spoiler_log_path.ends_with(".json") && !spoiler_log_path.ends_with('/'))
        spoiler_log_path += "/";

    // Add the filename afterwards
    if(!output_rom_path.empty() && *output_rom_path.rbegin() == '/')
        output_rom_path += options.hash_sentence() + ".md";
    if(!spoiler_log_path.empty() && *spoiler_log_path.rbegin() == '/')
        spoiler_log_path += options.hash_sentence() + ".json";
}

Json randomize(md::ROM& rom, RandomizerWorld& world, RandomizerOptions& options, PersonalSettings& personal_settings, const ArgumentDictionary& args)
{
    Json spoiler_json;

    spoiler_json["permalink"] = options.permalink();
    spoiler_json["hashSentence"] = options.hash_sentence();
    spoiler_json.merge_patch(options.to_json());

    // Apply randomizer options to alter World and RandomizerWorld accordingly before starting the actual randomization
    apply_randomizer_options(options, world);

    // Parse a potential "world" section inside the preset for plandos & half plandos
    WorldJsonParser::parse_world_json(world, options.world_json());

    // In rando mode, we rock our little World and shuffle things around to make a brand new experience on each seed.
    std::cout << "\nRandomizing world...\n";
    WorldShuffler shuffler(world, options);
    shuffler.randomize();

    // Apply patches to the game ROM to alter various things that are not directly part of the game world randomization
    std::cout << "Applying game patches...\n\n";
    apply_randomizer_patches(rom, world, options, personal_settings);

    std::string debug_log_path = args.get_string("debuglog");
    if (!debug_log_path.empty())
    {
        std::ofstream debug_log_file(debug_log_path);
        debug_log_file << shuffler.debug_log_as_json().dump(4);
        debug_log_file.close();
    }

    spoiler_json.merge_patch(SpoilerWriter::build_spoiler_json(world, options));
    spoiler_json["playthrough"] = shuffler.playthrough_as_json();

    // Output model if requested
    if(args.get_boolean("dumpmodel"))
    {
        if(options.allow_spoiler_log())
        {
            std::cout << "Outputting model...\n\n";
            ModelWriter::write_world_model(world);
            ModelWriter::write_logic_model(world);
            std::cout << "Model dumped to './json_data/'" << std::endl;
        }
        else
            std::cout << "Dumping model is not authorized on seeds with spoiler log disabled, it won't be generated.\n\n";
    }

    return spoiler_json;
}

void generate(const ArgumentDictionary& args)
{
    // Parse options from command-line args, preset file, plando file...
    RandomizerOptions options(args);
    PersonalSettings personal_settings(args);

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
    rom->mark_empty_chunk(0x19314, 0x19514); // Empty space
    rom->mark_empty_chunk(0x11F380, 0x120000); // Empty space
    rom->mark_empty_chunk(0x1FFAC0, 0x200000); // Empty space
    rom->mark_empty_chunk(0x2A442, 0x2A840); // Debug menu code & data
    rom->mark_empty_chunk(0x148AB6, 0x14AA78); // Unused bird sprite

    std::cout << "Settings: " << options.to_json().dump(2) << "\n\n";

    std::cout << "Permalink: " << options.permalink() << "\n";
    std::cout << "Share the permalink above with other people to enable them building the exact same seed.\n" << std::endl;

    RandomizerWorld world(*rom);

    Json spoiler_json = randomize(*rom, world, options, personal_settings, args);
    
    // Output world to ROM and save ROM unless it was explicitly specified by the user not to output a ROM
    if(!output_rom_path.empty())
    {
        std::cout << "Writing world to ROM...\n";
        world.write_to_rom(*rom);

        std::ofstream output_rom_file(output_rom_path, std::ios::binary);
        if(!output_rom_file)
            throw LandstalkerException("Could not open output ROM file for writing at path '" + output_rom_path + "'");

        rom->write_to_file(output_rom_file);
        std::cout << (rom->remaining_empty_bytes()/1000) << "Ko remaining of empty data" << std::endl;
        std::cout << "Randomized rom outputted to \"" << output_rom_path << "\".\n";
    }

    // Write a spoiler log to help the player
    if(!spoiler_log_path.empty())
    {
        if(options.allow_spoiler_log())
        {
            std::ofstream spoiler_file(spoiler_log_path);
            if(!spoiler_file)
                throw LandstalkerException("Could not open output log file for writing at path '" + spoiler_log_path + "'");

            spoiler_file << spoiler_json.dump(4);
            spoiler_file.close();
            std::cout << "Spoiler log written into \"" << spoiler_log_path << "\".\n";
        }
        else
            std::cout << "Spoiler log is not authorized under these settings, it won't be generated.\n";
    }
}

int main(int argc, char* argv[])
{
    int return_code = EXIT_SUCCESS;

    ArgumentDictionary args(argc, argv);

    std::cout << "======== Randstalker v" << RELEASE << " ========\n\n";

    if(args.contains("permalink") && args.get_string("permalink").empty())
    {
        std::string permalink;
        std::cout << "Please specify a permalink: ";
        std::getline(std::cin, permalink);
        args.set_string("permalink", permalink);
    }

    try
    {
        int seed_count = args.get_integer("seedcount", 1);
        for(int i=0 ; i<seed_count ; ++i)
            generate(args);
    }
    catch(LandstalkerException& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return_code = EXIT_FAILURE;
    }

    if(args.get_boolean("pause", true))
    {
        std::cout << "\nPress any key to exit.";
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    return return_code;
}
