#include "RandomizerOptions.hpp"
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
			_seedAsString = _optionsDictionary["seed"];
		} 
		catch (std::invalid_argument&)
		{
			_seed = 0;
			for (uint32_t i = 0; i < seedString.length(); ++i)
				_seed += ((uint32_t)seedString[i]) * (i*100+1);

			_seedAsString = "\"" + _optionsDictionary["seed"] + "\" (" + Tools::stringify(_seed) + ")";
		}
	} 
	else
	{
		_seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
		_seedAsString = Tools::stringify(_seed);
	}

	// Technical options
	_inputRomPath			= parseStringOption("inputrom", "input.md");
	_outputRomPath			= parseStringOption("outputrom", "output.md");
	_spoilerLogPath			= parseStringOption("outputlog", "spoiler.log");
	_debugLogPath			= parseStringOption("debuglog", "debug.log");
	_pauseAfterGeneration	= parseBooleanOption("pause", true);

	// Gameplay options
	_armorUpgrades			= parseBooleanOption("armorupgrades", true);
	_shuffleTiborTrees		= parseBooleanOption("shuffletrees", false);
	_saveAnywhereBook		= parseBooleanOption("recordbook", true);
	_spawnLocation			= parseSpawnLocationEnumOption("spawnlocation", SpawnLocation::MASSAN);
	_dungeonSignHints		= parseBooleanOption("dungeonsignhints", false);

	// Miscellaneous options
	_addIngameItemTracker	= parseBooleanOption("ingametracker", false);
	_hudColor				= parseStringOption("hudcolor", "default");
}

std::string RandomizerOptions::getSpawnLocationAsString() const
{
	switch (_spawnLocation)
	{
	case SpawnLocation::MASSAN:		return "Massan";
	case SpawnLocation::GUMI:		return "Gumi";
	case SpawnLocation::RYUMA:		return "Ryuma";
	case SpawnLocation::RANDOM:		return "Random";
	}
	return "???";
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