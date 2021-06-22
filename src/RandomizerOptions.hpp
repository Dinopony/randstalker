#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.hpp"
#include "Enums/SpawnLocations.hpp"

class RandomizerOptions 
{
public:
	RandomizerOptions(int argc, char* argv[]);

	const std::string& getSeedString() const { return _seedAsString; }
	uint32_t getSeed() const { return _seed; }

	const std::string& getInputROMPath() const { return _inputRomPath; }
	const std::string& getOutputROMPath() const { return _outputRomPath; }
	const std::string& getSpoilerLogPath() const { return _spoilerLogPath; }
	const std::string& getDebugLogPath() const { return _debugLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }

	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	SpawnLocation getSpawnLocation() const { return _spawnLocation;  }
	bool fillDungeonSignsWithHints() const { return _dungeonSignHints; }

	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	const std::string getHUDColor() const { return _hudColor; }

	std::string getSpawnLocationAsString() const;

private:
	void parseOptionsDictionaryFromArgs(int argc, char* argv[]);
	std::string parseStringOption(const std::string& name, const std::string& defaultValue) const;
	bool parseBooleanOption(const std::string& name, bool defaultValue) const;
	SpawnLocation parseSpawnLocationEnumOption(const std::string& name, SpawnLocation defaultValue) const;

	// ------------- Attributes -------------

	std::map<std::string, std::string> _optionsDictionary;

	std::string _seedAsString;
	uint32_t _seed;
	
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	std::string _debugLogPath;
	bool _pauseAfterGeneration;

	bool _armorUpgrades;
	bool _shuffleTiborTrees;
	bool _saveAnywhereBook;
	SpawnLocation _spawnLocation;
	bool _dungeonSignHints;

	bool _addIngameItemTracker;
	std::string _hudColor;

	// record book in inventory / record book findable in chests / record book disabled
};
