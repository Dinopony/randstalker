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
#include "SpoilerLog.hpp"
#include "ArgumentDictionary.hpp"
#include "Globals.hpp"
#include "Exceptions.hpp"

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
		RandomizerOptions options(argsDictionary);

		md::ROM* rom = getInputROM(options.getInputROMPath());
	
		// Perform game changes unrelated with the randomization part. This is mostly changing the game mechanics or altering slightly
		// a few maps by removing or changing NPCs.
		alterRomBeforeRandomization(*rom, options);
		
		// Create a replica model of Landstalker world, randomize it and save it to the ROM	
		World world(options);
		WorldRandomizer randomizer(world, options);
		randomizer.randomize();
		world.writeToROM(*rom);

		// Write a spoiler log to help the player
		SpoilerLog(options, world).writeToFile();

		// Perform game changes after the actual randomization. This is usually required when we need to point on a data block
		// which we don't know the exact position before it is actually written.
		alterRomAfterRandomization(*rom, options);

		rom->saveAs(options.getOutputROMPath());

		std::cout << "Randomized rom outputted to \"" << options.getOutputROMPath() << "\".\n" << std::endl;
		std::cout << "Permalink: \"" << options.toPermalink() << "\"" << std::endl;
		std::cout << "Share this permalink with other people to enable them building the exact same seed.\n" << std::endl;

		if ( options.mustPause() )
		{
			std::cout << "Press any key to exit.";
			std::string dummy;
			std::getline(std::cin, dummy);
		}
	} 
	catch(RandomizerException& e) 
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
