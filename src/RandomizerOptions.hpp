#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>
#include "Tools.hpp"
#include "Enums/SpawnLocations.hpp"
#include "ArgumentDictionary.hpp"
#include "Extlibs/json.hpp"

constexpr double DEFAULT_FILLING_RATE = 0.10;

class RandomizerOptions 
{
public:
	RandomizerOptions(const ArgumentDictionary& args);
	
	void parsePermalink(const std::string& permalink);
	void parseJsonPresetFile(const std::string& presetPath);
	void parseSettingsArguments(const ArgumentDictionary& args);
	void parsePersonalArguments(const ArgumentDictionary& args);
	void validate();

	uint32_t getSeed() const { return _seed; }
	std::vector<std::string> getHashWords() const;
	std::string getHashSentence() const { return Tools::join(this->getHashWords(), " "); }
	std::string getPermalink() const;

	void print(std::ostream& stream) const;
	void printPersonalSettings(std::ostream& stream) const;
	nlohmann::json toJSON() const;

	// Seed-related options (included in permalink)
	SpawnLocation getSpawnLocation() const;
	uint8_t getJewelCount() const { return _jewelCount; }
	double getFillingRate() const { return _fillingRate; }
	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool useRecordBook() const { return _saveAnywhereBook; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool fillDungeonSignsWithHints() const { return _dungeonSignHints; }
	bool allowSpoilerLog() const { return _allowSpoilerLog; }

	// Personal options (not included in permalink)
	const std::string& getInputROMPath() const { return _inputRomPath; }
	std::string getOutputROMPath() const { return _outputRomPath; }
	std::string getSpoilerLogPath() const { return _spoilerLogPath; }
	const std::string& getDebugLogPath() const { return _debugLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }
	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	const std::string getHUDColor() const { return _hudColor; }

private:
	// ------------- Preset Settings -------------
	uint32_t _seed;
	SpawnLocation _spawnLocation;
	uint32_t _jewelCount;
	double _fillingRate;
	bool _armorUpgrades;
	bool _saveAnywhereBook;
	bool _shuffleTiborTrees;
	bool _dungeonSignHints;
	bool _allowSpoilerLog;

	// ------------- Personal Settings -------------
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	std::string _debugLogPath;
	bool _pauseAfterGeneration;
	bool _addIngameItemTracker;
	std::string _hudColor;
};
