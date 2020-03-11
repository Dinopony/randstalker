#include "RandomizerOptions.h"
#include <set>
#include <unordered_map>

RandomizerOptions::RandomizerOptions(int argc, char* argv[])
{
	this->parseOptionsDictionaryFromArgs(argc, argv);

	if (_optionsDictionary.count("seed") && _optionsDictionary["seed"] != "random")
	{
		std::string& seedString = _optionsDictionary["seed"];
		try
		{
			_seed = (uint32_t) std::stoul(seedString);
		} 
		catch (std::invalid_argument&)
		{
			_seed = 0;
			for (uint32_t i = 0; i < seedString.length(); ++i)
				_seed += ((uint32_t)seedString[i]) * (i*100+1);
		}
	} 
	else
	{
		_seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
	}

	// Technical options
	_inputRomPath			= parseStringOption("inputrom", "input.md");
	_outputRomPath			= parseStringOption("outputrom", "output.md");
	_spoilerLogPath			= parseStringOption("outputlog", "spoiler.log");
	_pauseAfterGeneration	= parseBooleanOption("pause", true);

	// Gameplay options
	_armorUpgrades			= parseBooleanOption("armorupgrades", true);
	_shuffleTiborTrees		= parseBooleanOption("shuffletrees", false);
	_saveAnywhereBook		= parseBooleanOption("recordbook", true);
	_spawnLocation			= parseSpawnLocationEnumOption("spawnlocation", SpawnLocation::MASSAN);

	// Miscellaneous options
	_addIngameItemTracker	= parseBooleanOption("ingametracker", false);
	_hudColor				= parseStringOption("hudcolor", "default");
}

void RandomizerOptions::logToFile(std::ofstream& logFile) const
{
	logFile << "Seed: " << _seed << "\n\n";

	logFile << "Armor upgrades: " << ((_armorUpgrades) ? "enabled" : "disabled") << "\n";
	logFile << "Randomized Tibor trees: " << ((_shuffleTiborTrees) ? "enabled" : "disabled") << "\n";
	logFile << "Record Book: " << ((_saveAnywhereBook) ? "enabled" : "disabled") << "\n";
	logFile << "In-game item tracker: " << ((_addIngameItemTracker) ? "enabled" : "disabled") << "\n";

	logFile << "Starting location: ";
	if (_spawnLocation == SpawnLocation::MASSAN)			logFile << "Massan";
	else if (_spawnLocation == SpawnLocation::GUMI)			logFile << "Gumi";
	else if (_spawnLocation == SpawnLocation::RYUMA)		logFile << "Ryuma";
	else if (_spawnLocation == SpawnLocation::RANDOM)		logFile << "Random";
	logFile << "\n";

	logFile << "\n";
}

void RandomizerOptions::parseOptionsDictionaryFromArgs(int argc, char* argv[])
{
	for (int i = 1; i < argc; ++i)
	{
		std::string param = argv[i];
		if (param[0] != '-' || param[1] != '-')
			continue;

		auto tokenIter = std::find(param.begin() + 2, param.end(), '=');
		std::string paramName(param.begin() + 2, tokenIter);
		Tools::toLower(paramName);
		if (tokenIter != param.end())
			_optionsDictionary[paramName] = std::string(tokenIter + 1, param.end());
		else
			_optionsDictionary[paramName] = "";
	}
}

std::string RandomizerOptions::parseStringOption(const std::string& name, const std::string& defaultValue) const
{
	try
	{
		return _optionsDictionary.at(name);
	}
	catch (std::out_of_range&)
	{
		return defaultValue;
	}
}

bool RandomizerOptions::parseBooleanOption(const std::string& name, bool defaultValue) const
{
	// "--noParam" <==> "--param=false"
	std::string negationParam = "no" + name;
	if (_optionsDictionary.count(negationParam))
		return false;

	try
	{
		std::string contents = _optionsDictionary.at(name);
		Tools::toLower(contents);

		// "--param=false"
		if (contents == "false")
			return false;

		// "--param=true" or "--param"
		return true;
	}
	catch (std::out_of_range&)
	{
		// No trace of "--param" or "--noParam", return default value
		return defaultValue;
	}
}

SpawnLocation RandomizerOptions::parseSpawnLocationEnumOption(const std::string& name, SpawnLocation defaultValue) const
{
	const std::unordered_map<std::string, SpawnLocation> assocMap = {
		{ "massan", SpawnLocation::MASSAN	},
		{ "0",		SpawnLocation::MASSAN	},
		{ "gumi",	SpawnLocation::GUMI		},
		{ "1",		SpawnLocation::GUMI		},
		{ "ryuma",	SpawnLocation::RYUMA	},
		{ "2",		SpawnLocation::RYUMA	},
		{ "random",	SpawnLocation::RANDOM	},
		{ "3",		SpawnLocation::RANDOM	}
	};

	try
	{
		std::string spawnLoc = _optionsDictionary.at(name);
		Tools::toLower(spawnLoc);
		return assocMap.at(spawnLoc);
	}
	catch (std::out_of_range&)
	{
		return defaultValue;
	}
}