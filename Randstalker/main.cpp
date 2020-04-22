//////////////////////////////////////////////////////////////////////////////////////////
//
//		RANDSTALKER ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//		Developed by:	Dinopony (@DinoponyRuns)
//		Version:		v1.0
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

#include "MegadriveTools/MdRom.h"
#include "GameAlterations.h"
#include "Tools.h"
#include "World.h"
#include "WorldRandomizer.h"
#include "SpoilerLog.h"

constexpr auto RELEASE = "1.0";

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
	std::cout << "======== Randstalker v" << RELEASE << " ========\n\n";

	RandomizerOptions options(argc, argv);

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

	if ( options.mustPause() )
	{
		std::cout << "Press any key to exit.";
		std::string dummy;
		std::getline(std::cin, dummy);
	}

	return 0;
}
