#pragma once

#include <map>
#include <string>
#include <stdexcept>
#include <chrono>
#include <fstream>

#include "extlibs/json.hpp"

#include "tools/tools.hpp"
#include "tools/argument_dictionary.hpp"

constexpr double DEFAULT_FILLING_RATE = 0.10;

class RandomizerOptions 
{
public:
	RandomizerOptions();
	RandomizerOptions(const ArgumentDictionary& args);
	~RandomizerOptions();
	
	void parsePermalink(const std::string& permalink);
	void parseSettingsArguments(const ArgumentDictionary& args);
	void parsePersonalArguments(const ArgumentDictionary& args);
	void validate();

	Json to_json() const;
	void parseJSON(const Json& json);
	Json getPersonalSettingsAsJSON() const;
	
	// Game patching options 
	const std::vector<std::string>& getPossibleSpawnLocations() const { return _possibleSpawnLocations; }
	uint8_t getJewelCount() const { return _jewelCount; }
	bool useArmorUpgrades() const { return _armorUpgrades; }
	bool fillDungeonSignsWithHints() const { return _dungeonSignHints; }
	uint8_t getStartingLife() const { return _startingLife; }
	uint16_t getStartingGold() const { return _startingGold; }
	const std::map<std::string, uint8_t>& getStartingItems() const { return _startingItems; }
	const std::map<std::string, uint16_t>& getItemPrices() const { return _itemPrices; }
	bool fixArmletSkip() const { return _fixArmletSkip; }
	bool fixTreeCuttingGlitch() const { return _fixTreeCuttingGlitch; }
	const std::map<std::string, uint8_t>& getItemMaxQuantities() const { return _itemMaxQuantities; }
	bool consumableRecordBook() const { return _consumableRecordBook; }

	// Randomization options
	uint32_t getSeed() const { return _seed; }
	bool allowSpoilerLog() const { return _allowSpoilerLog; }
	double getFillingRate() const { return _fillingRate; }
	bool shuffleTiborTrees() const { return _shuffleTiborTrees; }
	bool handleGhostJumpingInLogic() const { return _ghostJumpingInLogic; }
	bool hasCustomMandatoryItems() const { return _mandatoryItems != nullptr; }
	const std::map<std::string, uint16_t>& getMandatoryItems() const { return *_mandatoryItems; }
	bool hasCustomFillerItems() const { return _fillerItems != nullptr; }
	const std::map<std::string, uint16_t>& getFillerItems() const { return *_fillerItems; }

	std::vector<std::string> getHashWords() const;
	std::string getHashSentence() const { return Tools::join(this->getHashWords(), " "); }
	std::string getPermalink() const;

	// Personal options 
	const std::string& getInputROMPath() const { return _inputRomPath; }
	std::string getOutputROMPath() const { return _outputRomPath; }
	std::string getSpoilerLogPath() const { return _spoilerLogPath; }
	const std::string& getDebugLogPath() const { return _debugLogPath; }
	bool mustPause() const { return _pauseAfterGeneration; }
	bool addIngameItemTracker() const { return _addIngameItemTracker; }
	const std::string getHUDColor() const { return _hudColor; }

	// Plando-specific options
	bool isPlando() const { return _plandoEnabled; }
	const Json& getInputPlandoJSON() const { return _plandoJSON; }

private:
	// ------------- Game patching settings -------------
	// (included in permalink, presets & plandos)
	uint32_t _jewelCount;
	bool _armorUpgrades;
	bool _dungeonSignHints;
	uint8_t _startingLife;
	uint16_t _startingGold;
	std::map<std::string, uint8_t> _startingItems;
	std::map<std::string, uint16_t> _itemPrices;
	bool _fixArmletSkip;
	bool _fixTreeCuttingGlitch;
	std::map<std::string, uint8_t> _itemMaxQuantities;
	bool _consumableRecordBook;
	
	// ------------- Randomization settings -------------
	// (included in permalink & presets, not in plandos)
	uint32_t _seed;
	bool _allowSpoilerLog;
	double _fillingRate;
	std::vector<std::string> _possibleSpawnLocations;
	bool _shuffleTiborTrees;
	bool _ghostJumpingInLogic;
	std::map<std::string, uint16_t>* _mandatoryItems;
	std::map<std::string, uint16_t>* _fillerItems;

	// ------------- Personal settings -------------
	// (not included in permalink nor presets)
	std::string _inputRomPath;
	std::string _outputRomPath;
	std::string _spoilerLogPath;
	std::string _debugLogPath;
	bool _pauseAfterGeneration;
	bool _addIngameItemTracker;
	std::string _hudColor;

	// Plando-specific arguments
	bool _plandoEnabled;
	Json _plandoJSON;
};