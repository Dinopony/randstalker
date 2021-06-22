#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.hpp"
#include "Enums/SpawnLocations.hpp"
#include "ArgumentDictionary.hpp"

class RandomizerOptions 
{
public:
	RandomizerOptions(const ArgumentDictionary& args);

	const std::string& getSeedString() const { return _seedAsString; }
	uint32_t getSeed() const { return _seed; }
	std::string toPermalink() const;
	
	// Personal options (not included in permalink)
	const std::string& getInputROMPath() const { return _inputRomPath; }
	const std::string& getOutputROMPath() const { return _outputRomPath; }
	const std::string& getSpoilerLogPath() const { return _spoilerLogPath; }
	const std::string& getDebugLogPath() const { return _debugLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }
	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	const std::string getHUDColor() const { return _hudColor; }

	// Seed-related options (included in permalink)
	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	SpawnLocation getSpawnLocation() const { return _spawnLocation;  }
	bool fillDungeonSignsWithHints() const { return _dungeonSignHints; }
	bool allowSpoilerLog() const { return _allowSpoilerLog; }

private:
	// ------------- Attributes -------------
	std::string _seedAsString;
	uint32_t _seed;
	
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	std::string _debugLogPath;
	bool _pauseAfterGeneration;
	bool _addIngameItemTracker;
	std::string _hudColor;

	bool _armorUpgrades;
	bool _shuffleTiborTrees;
	bool _saveAnywhereBook;
	SpawnLocation _spawnLocation;
	bool _dungeonSignHints;
	bool _allowSpoilerLog;
};
