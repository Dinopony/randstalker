//////////////////////////////////////////////////////////////////////////////////////////
//
//     RANDSTALKER ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//        Developed by:    Dinopony (@DinoponyRuns)
//        Version:        v1.01
//
// ---------------------------------------------------------------------------------------
//
//  Thanks to the whole Landstalker speedrunning community for being supportive during the whole process of developing this
//  Special mention to Wizardwhosaysni for being extra helpful with his deep knowledge of Megadrive reverse-engineering
// 
//
//  Command line syntax:
//        randstalker [args]
//
//    Common parameters:
//        --permalink="value"        ===> Permalink from a previous generation, allowing you to build the exact same seed
//        --inputRom="value"        ===> Path to the game ROM used as input for the randomization (this file will only be read, not modified).
//        --outputRom="value"        ===> Path where the randomized ROM will be put, defaults to 'output.md' in current working directory.
//        --seed="value"            ===> Random seed (integer value or "random") used to alter the game. Using the same seed twice will produce the same result.
//        --outputLog="value"        ===> Path where the seed log will be put, defaults to 'randstalker.log' in current working directory.
//        --noPause                ===> Don't ask to press a key at the end of generation (useful for automated generation systems)
//
//    Randomization options:
//        --fillingRate            ===> Set the randomizing algorithm step filling rate from 0.0 to 1.0 (default 0.20)
//        --shuffleTrees            ===> Randomize Tibor trees
//        --noArmorUpgrades        ===> Don't use armor upgrades, just place vanilla armors randomly
//        --spawnLocation            ===> Spawn point between Massan, Gumi and Ryuma
//        --noRecordBook            ===> Record Book not available in inventory
//        --dungeonSignHints        ===> Whether to add extra hints on signs inside dungeons (e.g. Thieves Hideout, Mir Tower...)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <iostream>

#include "extlibs/base64.hpp"

#include "model/world_region.hpp"
#include "model/item_source.hpp"

#include "patches/patches.hpp"

#include "tools/megadrive/rom.hpp"
#include "tools/argument_dictionary.hpp"
#include "tools/tools.hpp"

#include "exceptions.hpp"
#include "world.hpp"
#include "world_randomizer.hpp"

md::ROM* getInputROM(std::string inputRomPath)
{
    md::ROM* rom = new md::ROM(inputRomPath);
    while (!rom->is_valid())
    {
        delete rom;
        if (!inputRomPath.empty())
            std::cout << "[ERROR] ROM input path \"" << inputRomPath << "\" is wrong, and no ROM could be opened this way.\n\n";
        std::cout << "Please specify input ROM path (or drag ROM on Randstalker.exe icon before launching): ";
        std::getline(std::cin, inputRomPath);
        rom = new md::ROM(inputRomPath);
    }

    return rom;
}

void process_paths(const ArgumentDictionary& args, const RandomizerOptions& options,
                    std::string& input_rom_path, std::string& output_rom_path, std::string& spoiler_log_path)
{
    input_rom_path = args.get_string("inputrom", "./input.md");
    output_rom_path = args.get_string("outputrom", "./");
    spoiler_log_path = args.get_string("outputlog");

   // Clean output ROM path and determine if it's a directory or a file
    bool output_path_is_a_directory = true;
    if(!output_rom_path.empty())
    {
        output_path_is_a_directory = !Tools::endsWith(output_rom_path, ".md") && !Tools::endsWith(output_rom_path, ".bin");
        if(output_path_is_a_directory && *output_rom_path.rbegin() != '/')
            output_rom_path += "/";
    }

    // Clean output log path and if it wasn't specified, give it an appropriate default value
    if(spoiler_log_path.empty())
    {
        if(output_path_is_a_directory && !output_rom_path.empty())
            spoiler_log_path = output_rom_path; // outputRomPath points to a directory, use the same for the spoiler log
        else
            spoiler_log_path = "./"; // outputRomPath points to a file, use cwd for the spoiler log
    }
    if(!Tools::endsWith(spoiler_log_path, ".json") && *spoiler_log_path.rbegin() != '/')
        spoiler_log_path += "/";

    // Add the filename afterwards
    if(!output_rom_path.empty() && *output_rom_path.rbegin() == '/')
        output_rom_path += options.hash_sentence() + ".md";
    if(*spoiler_log_path.rbegin() == '/')
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

void displayOptions(const RandomizerOptions& options)
{
    Json optionsAsJSON = options.to_json();
    if(!options.has_custom_mandatory_items())
        optionsAsJSON["randomizerSettings"]["mandatoryItems"] = "default";
    if(!options.has_custom_filler_items())
        optionsAsJSON["randomizerSettings"]["fillerItems"] = "default";
    std::cout << "Settings: " << optionsAsJSON.dump(2) << "\n\n";
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
    md::ROM* rom = getInputROM(input_rom_path);
    rom->mark_empty_chunk(0x11F380, 0x120000);
    rom->mark_empty_chunk(0x1FFAC0, 0x200000);

    World world(*rom, options);

    displayOptions(options);

    Json spoiler_json;
    if(options.is_plando())
    {
        spoiler_json = plandomize(world, options, args);
    }
    else
    {
        spoiler_json = randomize(world, options, args);
    }

    std::cout << "Writing world to ROM...\n";
    world.write_to_rom(*rom);

    // Apply patches to the game ROM to alter various things that are not directly part of the game world randomization
    std::cout << "Applying game patches...\n\n";
    patch_game_init(*rom, options, world);
    patch_story_flag_reading(*rom, options, world);
    patch_item_behavior(*rom, options, world);
    apply_other_patches(*rom, options, world);

    if(!output_rom_path.empty())
    {
        rom->save_as(output_rom_path);
        std::cout << "Randomized rom outputted to \"" << output_rom_path << "\".\n\n";
    }

    // Write a spoiler log to help the player
    if(!spoiler_log_path.empty())
    {
        if(options.allow_spoiler_log())
        {
            std::ofstream spoilerFile(spoiler_log_path);
            if (spoilerFile)
                spoilerFile << spoiler_json.dump(4);
            spoilerFile.close();
            std::cout << "Spoiler log written into \"" << spoiler_log_path << "\".\n\n";
        }
        else
            std::cout << "Spoiler log is not authorized under these settings, it won't be generated.\n\n";
    }
}

int main(int argc, char* argv[])
{
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
        return 1;
    }

    if(args.get_boolean("pause", true))
    {
        std::cout << "Press any key to exit.";
        std::string dummy;
        std::getline(std::cin, dummy);
    }

    return 0;
}
