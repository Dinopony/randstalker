#include "RandomizerOptions.hpp"
#include "Extlibs/Base64.hpp"
#include "Exceptions.hpp"
#include "Tools.hpp"
#include <iostream>

RandomizerOptions::RandomizerOptions(const ArgumentDictionary& args)
{
	std::string permalinkString = args.getString("permalink");
	if(!permalinkString.empty()) 
	{
		try {
			// Permalink case: extract seed related options from the decoded permalink
			std::string decodedPermalink = base64_decode(permalinkString.substr(1, permalinkString.size()-2));
			std::vector<std::string> tokens = Tools::split(decodedPermalink, ";");

			std::string releaseVersion = tokens.at(0);
			if(releaseVersion != MAJOR_RELEASE) {
				throw WrongVersionException("This permalink comes from an incompatible version of Randstalker (" + releaseVersion + ").");
			}

			_seed = std::stoul(tokens.at(1));

			_jewelCount = std::stoi(tokens.at(2));
			_fillingRate = std::stod(tokens.at(3));
			_armorUpgrades = (tokens.at(4) == "t");
			_shuffleTiborTrees = (tokens.at(5) == "t");
			_saveAnywhereBook = (tokens.at(6) == "t");
			_spawnLocation = spawnLocationFromString(tokens.at(7));
			_dungeonSignHints = (tokens.at(8) == "t");
			_allowSpoilerLog = (tokens.at(9) == "t");
		} 
		catch(std::exception&) {
			throw RandomizerException("Invalid permalink given.");
		}
	} 
	else 
	{
		// New seed case: extract seed related options from the CLI arguments
		std::string seedString = args.getString("seed", "random");
		try {
			_seed = (uint32_t) std::stoul(seedString);
		} catch (std::invalid_argument&) {
			_seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
			_seed *= _seed;
		}

		// Seed-related options (included in permalink)  
		_jewelCount 			= args.getInteger("jewelcount", 2);
		_fillingRate			= args.getDouble("fillingrate", DEFAULT_FILLING_RATE);
		_armorUpgrades			= args.getBoolean("armorupgrades", true);
		_shuffleTiborTrees		= args.getBoolean("shuffletrees", false);
		_saveAnywhereBook		= args.getBoolean("recordbook", true);
		_spawnLocation			= spawnLocationFromString(args.getString("spawnlocation", "random"));
		_dungeonSignHints		= args.getBoolean("dungeonsignhints", false);
		_allowSpoilerLog		= args.getBoolean("allowSpoilerLog", true);
	}

	// Personal options (not included in permalink)
	_inputRomPath			= args.getString("inputrom", "./input.md");
	
	_outputRomPath = args.getString("outputrom", "./");
	bool points_to_file = Tools::endsWith(_outputRomPath, ".md") || Tools::endsWith(_outputRomPath, ".bin");
	if(!points_to_file && *_outputRomPath.rbegin() != '/')
		_outputRomPath += "/";

	_spoilerLogPath = args.getString("outputlog", "");
	if(_spoilerLogPath == "")
	{
		if(points_to_file)
			_spoilerLogPath = "./"; // outputRomPath points to a file, use cwd for the spoiler log
		else
			_spoilerLogPath = _outputRomPath; // outputRomPath points to a directory, use the same for the spoiler log
	}

	if(!Tools::endsWith(_spoilerLogPath, ".log") && *_spoilerLogPath.rbegin() != '/')
		_spoilerLogPath += "/";

	_debugLogPath			= args.getString("debuglog", "./debug.log");
	_pauseAfterGeneration	= args.getBoolean("pause", true);
	_addIngameItemTracker	= args.getBoolean("ingametracker", false);
	_hudColor				= args.getString("hudcolor", "default");
}

std::vector<std::string> RandomizerOptions::getHashWords() const
{
	std::vector<std::string> words = {
		"EkeEke", "Nail", "Horn", "Fang", "Magic", "Ice", "Thunder", "Gaia", "Fireproof", "Iron", "Spikes", "Healing", "Snow",
		"Mars", "Moon", "Saturn", "Venus", "Detox", "Statue", "Golden", "Mind", "Repair", "Casino", "Ticket", "Axe", "Ribbon",
		"Card", "Lantern", "Garlic", "Paralyze", "Chicken", "Death", "Jypta", "Sun", "Armlet", "Einstein", "Whistle", "Spell",
		"Book", "Lithograph", "Red", "Purple", "Jewel", "Pawn", "Ticket", "Gola", "Nole", "King", "Dragon", "Dahl", "Restoration",
		"Logs", "Oracle", "Stone", "Idol", "Key", "Safety", "Pass", "Bell", "Short", "Cake", "Life", "Stock", "Zak", "Duke",
		"Massan", "Gumi", "Ryuma", "Mercator", "Verla", "Destel", "Kazalt", "Greedly", "Dex", "Slasher", "Marley", "Nigel", "Friday",
		"Mir", "Miro", "Prospero", "Fara", "Orc", "Mushroom", "Slime", "Cyclops", "Ninja", "Ghost", "Tibor", "Knight", "Pockets",
		"Kado", "Kan", "Well", "Dungeon", "Loria", "Kayla", "Wally", "Ink", "Arthur", "Crypt", "Mummy", "Poison", "Labyrinth",
		"Palace", "Gold", "Waterfall", "Shrine", "Swamp", "Hideout", "Greenmaze", "Mines", "Lake", "Volcano", "Crate", "Jar", 
		"Helga", "Fahl", "Yard", "Twinkle", "Firedemon", "Spinner", "Golem", "Boulder"
	};

	std::mt19937 rng(_seed);
	Tools::shuffle(words, rng);
	return std::vector<std::string>(words.begin(), words.begin() + 4);
}

std::string RandomizerOptions::getPermalink() const
{
	std::ostringstream permalinkBuilder;
	permalinkBuilder 	<< MAJOR_RELEASE << ";"
						<< _seed << ";"
						<< _jewelCount << ";"
						<< (std::to_string(_fillingRate)) << ";"
						<< (_armorUpgrades ? "t" : "") << ";"
						<< (_shuffleTiborTrees ? "t" : "") << ";"
						<< (_saveAnywhereBook ? "t" : "") << ";"
						<< (spawnLocationToNumber(_spawnLocation)) << ";"
						<< (_dungeonSignHints ? "t" : "") << ";"
						<< (_allowSpoilerLog ? "t" : "");
	
	return "L" + base64_encode(permalinkBuilder.str()) + "S";
}
