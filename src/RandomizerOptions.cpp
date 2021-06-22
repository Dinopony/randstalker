#include "RandomizerOptions.hpp"
#include "Extlibs/Base64.hpp"
#include "Globals.hpp"
#include "Exceptions.hpp"
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
			_armorUpgrades = (tokens.at(2) == "t");
			_shuffleTiborTrees = (tokens.at(3) == "t");
			_saveAnywhereBook = (tokens.at(4) == "t");
			_spawnLocation = spawnLocationFromString(tokens.at(5));
			_dungeonSignHints = (tokens.at(6) == "t");
			_allowSpoilerLog = (tokens.at(7) == "t");
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
		_armorUpgrades			= args.getBoolean("armorupgrades", true);
		_shuffleTiborTrees		= args.getBoolean("shuffletrees", false);
		_saveAnywhereBook		= args.getBoolean("recordbook", true);
		_spawnLocation			= spawnLocationFromString(args.getString("spawnlocation", "massan"));
		_dungeonSignHints		= args.getBoolean("dungeonsignhints", false);
		_allowSpoilerLog		= args.getBoolean("allowSpoilerLog", true);
	}

	// Personal options (not included in permalink)
	_inputRomPath			= args.getString("inputrom", "input.md");
	_outputRomPath			= args.getString("outputrom", "output.md");
	_spoilerLogPath			= args.getString("outputlog", "spoiler.log");
	_debugLogPath			= args.getString("debuglog", "debug.log");
	_pauseAfterGeneration	= args.getBoolean("pause", true);
	_addIngameItemTracker	= args.getBoolean("ingametracker", false);
	_hudColor				= args.getString("hudcolor", "default");
}

std::string RandomizerOptions::toPermalink() const
{
	std::ostringstream permalinkBuilder;
	permalinkBuilder 	<< MAJOR_RELEASE << ";"
						<< _seed << ";"
						<< (_armorUpgrades ? "t" : "") << ";"
						<< (_shuffleTiborTrees ? "t" : "") << ";"
						<< (_saveAnywhereBook ? "t" : "") << ";"
						<< (spawnLocationToNumber(_spawnLocation)) << ";"
						<< (_dungeonSignHints ? "t" : "") << ";"
						<< (_allowSpoilerLog ? "t" : "");
	
	return "L" + base64_encode(permalinkBuilder.str()) + "S";
}
