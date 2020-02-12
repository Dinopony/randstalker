#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.h"

class RandomizerOptions 
{
public:
	RandomizerOptions(int argc, char* argv[]) :
		_seed					(0),

		_inputRomPath			("input.md"),
		_outputRomPath			("output.md"),
		_spoilerLogPath			("spoiler.log"),
		_pauseAfterGeneration	(true),

		_armorUpgrades			(true),
		_shuffleTiborTrees		(false),
		_saveAnywhereBook		(true),
		_randomSpawnPoint		(false)
	{
		std::map<std::string, std::string> options = parseOptionsDictionaryFromArgs(argc, argv);

		if (options.count("seed") && options["seed"] != "random")
		{
			std::string& seedString = options["seed"];
			try
			{
				_seed = (uint32_t)std::stoul(seedString);
			}
			catch (std::invalid_argument&)
			{
				for (uint32_t i = 0; i < seedString.length(); ++i)
					_seed += (uint32_t)seedString[i];
			}
		}
		else
		{
			_seed = (uint32_t)std::chrono::system_clock::now().time_since_epoch().count();
		}


		if (options.count("inputrom"))
			_inputRomPath = options["inputrom"];
		if (options.count("outputrom"))
			_outputRomPath = options["outputrom"];
		if (options.count("outputlog"))
			_spoilerLogPath = options["outputrom"];
		if (options.count("pause"))
			_pauseAfterGeneration = (options["pause"] == "true");


		if (options.count("armorupgrades"))
			_armorUpgrades = (options["armorupgrades"] == "true");
		if (options.count("shuffletrees"))
			_shuffleTiborTrees = (options["shuffletrees"] == "true");
		if (options.count("recordbook"))
			_saveAnywhereBook = (options["recordbook"] == "true");
		if (options.count("randomspawn"))
			_randomSpawnPoint = (options["randomspawn"] == "true");
	}

	void logToFile(std::ofstream& logFile) const
	{
		logFile << "Seed: " << _seed << "\n\n";

		logFile << "Armor upgrades: " << ((_armorUpgrades) ? "enabled" : "disabled") << "\n";
		logFile << "Randomized Tibor trees: " << ((_shuffleTiborTrees) ? "enabled" : "disabled") << "\n";
		logFile << "Record Book: " << ((_saveAnywhereBook) ? "enabled" : "disabled") << "\n";
		logFile << "Random spawn point: " << ((_randomSpawnPoint) ? "enabled" : "disabled") << "\n";

		logFile << "\n";
	}

	static std::map<std::string, std::string> parseOptionsDictionaryFromArgs(int argc, char* argv[])
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

	uint32_t getSeed() const { return _seed; }
	const std::string& getInputROMPath() const { return _inputRomPath; }
	const std::string& getOutputROMPath() const { return _outputRomPath; }
	const std::string& getSpoilerLogPath() const { return _spoilerLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }

	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	bool useRandomSpawnPoint() const { return _randomSpawnPoint;  }

private:
	uint32_t _seed;
	
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	bool _pauseAfterGeneration;

	bool _armorUpgrades;
	bool _shuffleTiborTrees;
	bool _saveAnywhereBook;
	bool _randomSpawnPoint;
	
	// record book in inventory / record book findable in chests / record book disabled
	// enable / disable in-game item tracker (disabled by default)
	// spawn location : massan / gumi / ryuma / random

};
