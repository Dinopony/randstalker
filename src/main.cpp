//////////////////////////////////////////////////////////////////////////////////////////
//
//		RANDSTALKER ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//		Developed by:	Dinopony (@DinoponyRuns)
//		Version:		v1.01
//
// ---------------------------------------------------------------------------------------
//
//	Thanks to the whole Landstalker speedrunning community for being supportive during the whole process of developing this
//  Special mention to Wizardwhosaysni for being extra helpful with his deep knowledge of Megadrive reverse-engineering
// 
//
//  Command line syntax:
//		randstalker [args]
//
//	Common parameters:
//		--permalink="value"		===> Permalink from a previous generation, allowing you to build the exact same seed
//		--inputRom="value"		===> Path to the game ROM used as input for the randomization (this file will only be read, not modified).
//		--outputRom="value"		===> Path where the randomized ROM will be put, defaults to 'output.md' in current working directory.
//		--seed="value"			===> Random seed (integer value or "random") used to alter the game. Using the same seed twice will produce the same result.
//		--outputLog="value"		===> Path where the seed log will be put, defaults to 'randstalker.log' in current working directory.
//		--noPause				===> Don't ask to press a key at the end of generation (useful for automated generation systems)
//
//	Randomization options:
//		--fillingRate			===> Set the randomizing algorithm step filling rate from 0.0 to 1.0 (default 0.20)
//		--shuffleTrees			===> Randomize Tibor trees
//		--noArmorUpgrades		===> Don't use armor upgrades, just place vanilla armors randomly
//		--spawnLocation			===> Spawn point between Massan, Gumi and Ryuma
//		--noRecordBook			===> Record Book not available in inventory
//		--dungeonSignHints		===> Whether to add extra hints on signs inside dungeons (e.g. Thieves Hideout, Mir Tower...)
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <string>
#include <iostream>

#include "MegadriveTools/MdRom.hpp"
#include "GameAlterations.hpp"
#include "Tools.hpp"
#include "World.hpp"
#include "WorldRandomizer.hpp"
#include "ArgumentDictionary.hpp"
#include "Exceptions.hpp"

#include "WorldRegion.hpp"
#include "ItemSources.hpp"

md::ROM* getInputROM(std::string inputRomPath)
{
	md::ROM* rom = new md::ROM(inputRomPath);
	while (!rom->isValid())
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

int main(int argc, char* argv[])
{
	ArgumentDictionary argsDictionary(argc, argv);

	std::cout << "======== Randstalker v" << RELEASE << " ========\n\n";

	try
	{
		// Parse options from command-line args, preset file, plando file...
		RandomizerOptions options(argsDictionary);
		options.print(std::cout);
		options.printPersonalSettings(std::cout);

		// Load input ROM and tag known empty chunks of data to know where to inject code / data
		md::ROM* rom = getInputROM(options.getInputROMPath());
		rom->markChunkAsEmpty(0x11F380, 0x120000);
		rom->markChunkAsEmpty(0x1FFAC0, 0x200000);

		// Create a replica model of Landstalker world, randomize or plandomize it and save it to the ROM	
		World world(options);
		if(options.isPlando())
		{
			std::cout << "Plandomizing world...\n\n";
			world.parseJSON(options.getInputPlandoJSON());
		}
		else
		{
			std::cout << "Randomizing world...\n\n";
			WorldRandomizer randomizer(world, options);
			randomizer.randomize();
		}
		world.writeToROM(*rom);

		// Apply patches to the game ROM to alter various things that are not directly part of the game world randomization
		std::cout << "Applying patches...\n\n";
		applyPatches(*rom, options, world);

		rom->saveAs(options.getOutputROMPath());
		std::cout << "Randomized rom outputted to \"" << options.getOutputROMPath() << "\".\n";

		// Write a spoiler log to help the player
		if(options.allowSpoilerLog() && !options.getSpoilerLogPath().empty())
		{
			Json json;
			if(!options.isPlando())
			{
				json["permalink"] = options.getPermalink();
				json["hashSentence"] = options.getHashSentence();
				json["seed"] = options.getSeed();
			}

			json.merge_patch(options.toJSON());
			json.merge_patch(world.toJSON());

			std::ofstream spoilerFile(options.getSpoilerLogPath());
			if (spoilerFile)
				spoilerFile << json.dump(4);
			spoilerFile.close();
		}
		
		std::cout << "Spoiler log written into \"" << options.getSpoilerLogPath() << "\".\n";

		std::cout << "Share the permalink above with other people to enable them building the exact same seed.\n" << std::endl;

		// TEMPORARY: Output current preset
		if (argsDictionary.getBoolean("writepreset"))
		{
			Json json = options.toJSON();

			std::ofstream presetFile("./preset.json");
			if(presetFile)
				presetFile << json.dump(4);
			presetFile.close();
		}

		if ( options.mustPause() )
		{
			std::cout << "Press any key to exit.";
			std::string dummy;
			std::getline(std::cin, dummy);
		}
	} 
	catch(RandomizerException& e) 
	{
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
