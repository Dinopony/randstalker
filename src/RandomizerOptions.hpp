#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.hpp"
#include "Enums/SpawnLocations.hpp"
#include "ArgumentDictionary.hpp"

constexpr double DEFAULT_FILLING_RATE = 0.20;

class RandomizerOptions 
{
public:
	RandomizerOptions(const ArgumentDictionary& args);

	uint32_t getSeed() const { return _seed; }
	std::vector<std::string> getHashWords() const;
	std::string getHashSentence() const { return Tools::join(this->getHashWords(), " "); }
	std::string getPermalink() const;
	
	// Personal options (not included in permalink)
	const std::string& getInputROMPath() const { return _inputRomPath; }
	std::string getOutputROMPath() const
	{ 
		if(*_outputRomPath.rbegin() == '/')
			return _outputRomPath + this->getHashSentence() + ".md";
		return _outputRomPath;
	}
	std::string getSpoilerLogPath() const
	{ 
		if(*_spoilerLogPath.rbegin() == '/')
			return _spoilerLogPath + this->getHashSentence() + ".log";
		return _spoilerLogPath;
	}

	const std::string& getDebugLogPath() const { return _debugLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }
	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	const std::string getHUDColor() const { return _hudColor; }

	// Seed-related options (included in permalink)
	uint8_t getJewelCount() const { return _jewelCount; }
	double getFillingRate() const { return _fillingRate; }
	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	SpawnLocation getSpawnLocation() const { return _spawnLocation;  }
	bool fillDungeonSignsWithHints() const { return _dungeonSignHints; }
	bool allowSpoilerLog() const { return _allowSpoilerLog; }

private:
	// ------------- Attributes -------------
	uint32_t _seed;
	
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	std::string _debugLogPath;
	bool _pauseAfterGeneration;
	bool _addIngameItemTracker;
	std::string _hudColor;

	uint8_t _jewelCount;
	double _fillingRate;
	bool _armorUpgrades;
	bool _shuffleTiborTrees;
	bool _saveAnywhereBook;
	SpawnLocation _spawnLocation;
	bool _dungeonSignHints;
	bool _allowSpoilerLog;
};
