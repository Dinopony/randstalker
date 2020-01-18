//////////////////////////////////////////////////////////////////////////////////////////
//
//		RANDSTALKER ROM GENERATOR
//
// ---------------------------------------------------------------------------------------
//
//		Developed by:	Dinopony (@DinoponyRuns)
//		Version:		v0.9a
//
// ---------------------------------------------------------------------------------------
//
//	Special thanks to the whole Landstalker speedrunning community for being supportive
//	and helpful during the whole process of developing this!
//
//////////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <iostream>

#include "GameROM.h"
#include "GameAlterations.h"
#include "World.h"

constexpr auto RELEASE = "0.9a";

/*
 * Command line syntax:
 *		randstalker [input_rom_path] [seed] [output_rom_path]
 *
 *  - input_rom_path	===> Path to the game ROM used as input for the randomization (this file will only be read, not modified).
 *	- seed				===> Random seed (integer) used to alter the game. Using the same seed twice will produce the same result.
 *	- output_rom_path	===> Path where the randomized ROM will be put, defaults to 'output.md' in current working directory.
 */

int main(int argc, char* argv[])
{
	std::cout << "======== Randstalker v" << RELEASE << " ========\n\n";

	std::string inputRomPath;
	uint32_t seed = 0;

	// First argument => input rom path
	if (argc >= 2)
		inputRomPath = argv[1];
	GameROM* rom = new GameROM(inputRomPath);
	while (!rom->isValid())
	{
		delete rom;
		if (!inputRomPath.empty())
			std::cout << "[ERROR] ROM input path \"" << inputRomPath << "\" is wrong, and no ROM could be opened this way.\n\n";
		std::cout << "Please specify input ROM path (or drag ROM on Randstalker.exe icon before launching): ";
		std::getline(std::cin, inputRomPath);
		rom = new GameROM(inputRomPath);
	}

	std::cout << "\n";

	// Second argument => seed
	if (argc >= 3)
	{
		try {
			seed = std::stoi(argv[2]);
		}
		catch (std::invalid_argument&) {
			std::cout << "Seed parameter '" << argv[2] << "' is non numerical and thus isn't a valid seed.\n";
			seed = 0;
		}
	}

	if (!seed)
	{
		std::string inputtedSeedString;
		std::cout << "Please specify the seed (leave blank for random seed): ";
		std::getline(std::cin, inputtedSeedString);
		try {
			seed = (uint32_t)std::stoi(inputtedSeedString);
		}
		catch (std::invalid_argument&) {
			seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
		}
	}
	std::cout << "Used seed: " << seed << "\n\n";

	// Perform game changes unrelated with the randomization part
	alterROM(*rom);

	// Create a replica model of Landstalker world, randomize it and save it to the ROM
	std::ofstream logFile("randstalker.log");
	World landstalkerWorld;
	landstalkerWorld.randomize(seed, logFile);
	landstalkerWorld.writeToROM(*rom);
	logFile.close();

	std::string outputRomPath = "output.md";
	if (argc >= 4)
		outputRomPath = argv[3];
	rom->saveAs(outputRomPath);

	std::cout << "Randomized rom outputted to 'output.md'.\n\n";

	std::cout << "Press any key to exit.";
	std::string dummy;
	std::getline(std::cin, dummy);

	return 0;
}