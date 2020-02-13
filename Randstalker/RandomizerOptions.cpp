#include "RandomizerOptions.h"
#include <set>

RandomizerOptions::RandomizerOptions(int argc, char* argv[]) :
	_seed(0),

	_inputRomPath("input.md"),
	_outputRomPath("output.md"),
	_spoilerLogPath("spoiler.log"),
	_pauseAfterGeneration(true),

	_armorUpgrades(true),
	_shuffleTiborTrees(false),
	_saveAnywhereBook(true),
	_addIngameItemTracker(false),
	_spawnLocation(SpawnLocation::MASSAN)
{
	std::map<std::string, std::string> options = parseOptionsDictionaryFromArgs(argc, argv);

	if (options.count("seed") && options["seed"] != "random")
	{
		std::string& seedString = options["seed"];
		try
		{
			_seed = (uint32_t) std::stoul(seedString);
		} catch (std::invalid_argument&)
		{
			for (uint32_t i = 0; i < seedString.length(); ++i)
				_seed += ((uint32_t)seedString[i]) * (i*100+1);
		}
	} 
	else
	{
		_seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
	}


	if (options.count("inputrom"))
		_inputRomPath = options["inputrom"];
	if (options.count("outputrom"))
		_outputRomPath = options["outputrom"];
	if (options.count("outputlog"))
		_spoilerLogPath = options["outputlog"];
	if (options.count("pause"))
		_pauseAfterGeneration = (options["pause"] == "true");


	if (options.count("armorupgrades"))
		_armorUpgrades = (options["armorupgrades"] == "true");
	if (options.count("shuffletrees"))
		_shuffleTiborTrees = (options["shuffletrees"] == "true");
	if (options.count("recordbook"))
		_saveAnywhereBook = (options["recordbook"] == "true");
	if (options.count("ingametracker"))
		_addIngameItemTracker = (options["ingametracker"] == "true");
	

	if (options.count("spawnlocation"))
	{
		std::string spawnLoc = options["spawnlocation"];
		Tools::toLower(spawnLoc);
		if (spawnLoc == "gumi")			_spawnLocation = SpawnLocation::GUMI;
		else if (spawnLoc == "ryuma")	_spawnLocation = SpawnLocation::RYUMA;
		else if (spawnLoc == "random")	_spawnLocation = SpawnLocation::RANDOM;
		else							_spawnLocation = SpawnLocation::MASSAN;
	}
	else if (options.count("randomspawn"))
		_spawnLocation = (options["randomspawn"] == "true") ? SpawnLocation::RANDOM : SpawnLocation::MASSAN;
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

std::map<std::string, std::string> RandomizerOptions::parseOptionsDictionaryFromArgs(int argc, char* argv[])
{
	std::map<std::string, std::string> options;

	for (int i = 1; i < argc; ++i)
	{
		std::string param = argv[i];
		if (param[0] != '-' || param[1] != '-')
			continue;

		auto tokenIter = std::find(param.begin() + 2, param.end(), '=');
		std::string paramName(param.begin() + 2, tokenIter);
		Tools::toLower(paramName);
		if (tokenIter != param.end())
			options[paramName] = std::string(tokenIter + 1, param.end());
		else if (paramName.substr(0, 2) == "no")
			options[paramName.substr(2)] = "false";
		else
			options[paramName] = "true";
	}

	return options;
}