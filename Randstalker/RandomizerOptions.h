#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.h"
#include "Constants/SpawnLocations.h"

class RandomizerOptions 
{
public:
	RandomizerOptions(int argc, char* argv[]);

	void logToFile(std::ofstream& logFile) const;

	uint32_t getSeed() const { return _seed; }

	const std::string& getInputROMPath() const { return _inputRomPath; }
	const std::string& getOutputROMPath() const { return _outputRomPath; }
	const std::string& getSpoilerLogPath() const { return _spoilerLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }

	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	SpawnLocation getSpawnLocation() const { return _spawnLocation;  }

	static std::map<std::string, std::string> parseOptionsDictionaryFromArgs(int argc, char* argv[]);

private:
	uint32_t _seed;
	
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	bool _pauseAfterGeneration;

	bool _armorUpgrades;
	bool _shuffleTiborTrees;
	bool _saveAnywhereBook;
	bool _addIngameItemTracker;
	SpawnLocation _spawnLocation;
	
	// record book in inventory / record book findable in chests / record book disabled
	// spawn location : massan / gumi / ryuma / random
};
