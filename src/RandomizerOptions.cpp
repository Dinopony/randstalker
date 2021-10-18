#include "RandomizerOptions.hpp"
#include "Extlibs/Base64.hpp"
#include "Exceptions.hpp"
#include "Tools.hpp"
#include <iostream>

RandomizerOptions::RandomizerOptions(const ArgumentDictionary& args) :
	_seed					(0),
	_spawnLocation			(SpawnLocation::RANDOM),
	_jewelCount				(2),
	_fillingRate			(DEFAULT_FILLING_RATE),
	_armorUpgrades			(true),
	_saveAnywhereBook		(true),
	_shuffleTiborTrees		(false), 
	_dungeonSignHints		(false),
	_allowSpoilerLog		(true),

	_inputRomPath			("./input.md"),
	_outputRomPath			("./"),
	_spoilerLogPath			(""),
	_debugLogPath			(""),
	_pauseAfterGeneration	(true),
	_addIngameItemTracker	(false),
	_hudColor				("default"),

	_plandoEnabled			(false),
	_plandoJSON				(nullptr)
{
	std::string plandoPath = args.getString("plando");
	if(!plandoPath.empty())
	{
		_plandoEnabled = true;
		std::ifstream plandoFile(plandoPath);
		if(!plandoFile)
			throw RandomizerException("Could not open plando file at given path '" + plandoPath + "'");
		
		std::cout << "Reading plando file '" << plandoPath << "'...\n\n";

		plandoFile >> _plandoJSON;
		this->parseJSON(_plandoJSON);
	}

	std::string permalinkString = args.getString("permalink");
	if(!permalinkString.empty() && !_plandoEnabled) 
	{
		this->parsePermalink(permalinkString);
	}
	else
	{
		// Parse seed from args, or generate a random one if it's missing
		std::string seedString = args.getString("seed", "random");
		try {
			_seed = (uint32_t) std::stoul(seedString);
		} catch (std::invalid_argument&) {
			_seed = (uint32_t) std::chrono::system_clock::now().time_since_epoch().count();
			_seed *= _seed;
		}

		std::string presetPath = args.getString("preset");
		if(!presetPath.empty() && !_plandoEnabled)
		{
			std::ifstream presetFile(presetPath);
			if(!presetFile)
				throw RandomizerException("Could not open preset file at given path '" + presetPath + "'");

			std::cout << "Reading preset file '" << presetPath << "'...\n\n";

			Json presetJson;
			presetFile >> presetJson;
			this->parseJSON(presetJson);
		}

		this->parseSettingsArguments(args);
	}

	this->parsePersonalArguments(args);
	this->validate();
}

void RandomizerOptions::parsePermalink(const std::string& permalink)
{
	try {
		// Permalink case: extract seed related options from the decoded permalink
		std::string decodedPermalink = base64_decode(permalink.substr(1, permalink.size()-2));
		std::vector<std::string> tokens = Tools::split(decodedPermalink, ";");

		std::string releaseVersion = tokens.at(0);
		if(releaseVersion != MAJOR_RELEASE) {
			throw WrongVersionException("This permalink comes from an incompatible version of Randstalker (" + releaseVersion + ").");
		}

		_seed = std::stoul(tokens.at(1));

		_jewelCount = std::stoi(tokens.at(2));
		_fillingRate = std::stod(tokens.at(3));
		_armorUpgrades = (tokens.at(4) == "t");
		_shuffleTiborTrees = (tokens.at(5) == "t");
		_saveAnywhereBook = (tokens.at(6) == "t");
		_spawnLocation = spawnLocationFromString(tokens.at(7));
		_dungeonSignHints = (tokens.at(8) == "t");
		_allowSpoilerLog = (tokens.at(9) == "t");
	}
	catch(std::exception&) {
		throw RandomizerException("Invalid permalink given.");
	}
}

void RandomizerOptions::parseSettingsArguments(const ArgumentDictionary& args)
{
	// Seed-related options (included in permalink)  
	if(args.contains("jewelcount"))			_jewelCount = args.getInteger("jewelcount");
	if(args.contains("fillingrate")) 		_fillingRate = args.getDouble("fillingrate");
	if(args.contains("armorupgrades")) 		_armorUpgrades = args.getBoolean("armorupgrades");
	if(args.contains("shuffletrees")) 		_shuffleTiborTrees = args.getBoolean("shuffletrees");
	if(args.contains("recordbook")) 		_saveAnywhereBook = args.getBoolean("recordbook");
	if(args.contains("dungeonsignhints")) 	_dungeonSignHints = args.getBoolean("dungeonsignhints");
	if(args.contains("allowSpoilerLog")) 	_allowSpoilerLog = args.getBoolean("allowSpoilerLog");
	if(args.contains("spawnlocation")) 		_spawnLocation = spawnLocationFromString(args.getString("spawnlocation"));
}

void RandomizerOptions::parsePersonalArguments(const ArgumentDictionary& args)
{
	// Personal options (not included in permalink)
	if(args.contains("inputrom"))			_inputRomPath = args.getString("inputrom");
	if(args.contains("outputrom"))			_outputRomPath = args.getString("outputrom");
	if(args.contains("outputlog"))			_spoilerLogPath = args.getString("outputlog");
	if(args.contains("debuglog"))			_debugLogPath = args.getString("debuglog");
	if(args.contains("pause"))				_pauseAfterGeneration = args.getBoolean("pause");
	if(args.contains("ingametracker"))		_addIngameItemTracker = args.getBoolean("ingametracker");	
	if(args.contains("hudcolor"))			_hudColor = args.getString("hudcolor");
}

Json RandomizerOptions::toJSON() const
{
	Json json;

	json["gameSettings"]["spawnLocation"] = spawnLocationToString(_spawnLocation);
	json["gameSettings"]["jewelCount"] = _jewelCount;
	json["gameSettings"]["armorUpgrades"] = _armorUpgrades;
	json["gameSettings"]["recordBook"] = _saveAnywhereBook;
	json["gameSettings"]["dungeonSignHints"] = _dungeonSignHints;

	json["randomizerSettings"]["fillingRate"] = _fillingRate;	
	json["randomizerSettings"]["shuffleTrees"] = _shuffleTiborTrees;
	
	return json;
}

void RandomizerOptions::parseJSON(const Json& json)
{
	if(json.contains("gameSettings"))
	{
		const Json& gameSettingsJson = json.at("gameSettings");

		if(gameSettingsJson.contains("spawnLocation"))
		{
			std::string spawnLocStr;
			gameSettingsJson.at("spawnLocation").get_to(spawnLocStr);
			_spawnLocation = spawnLocationFromString(spawnLocStr);
		}

		if(gameSettingsJson.contains("jewelCount"))			
			_jewelCount = gameSettingsJson.at("jewelCount");
		if(gameSettingsJson.contains("armorUpgrades"))
			_armorUpgrades = gameSettingsJson.at("armorUpgrades");
		if(gameSettingsJson.contains("recordBook"))	
			_saveAnywhereBook = gameSettingsJson.at("recordBook");
		if(gameSettingsJson.contains("dungeonSignHints"))
			_dungeonSignHints = gameSettingsJson.at("dungeonSignHints");
	}

	if(json.contains("randomizerSettings"))
	{
		const Json& randomizerSettingsJson = json.at("randomizerSettings");

		if(randomizerSettingsJson.contains("fillingRate"))
			_fillingRate = randomizerSettingsJson.at("fillingRate");
		if(randomizerSettingsJson.contains("shuffleTrees"))
			_shuffleTiborTrees = randomizerSettingsJson.at("shuffleTrees");
	}
}


void RandomizerOptions::validate()
{
	if(_jewelCount > 9)
		throw RandomizerException("Jewel count must be between 0 and 9.");

	// Clean output ROM path and determine if it's a directory or a file
	bool outputRomPathIsAFile = Tools::endsWith(_outputRomPath, ".md") || Tools::endsWith(_outputRomPath, ".bin");
	if(!outputRomPathIsAFile && *_outputRomPath.rbegin() != '/')
		_outputRomPath += "/";

	// Clean output log path and if it wasn't specified, give it an appropriate default value
	if(_spoilerLogPath.empty())
	{
		if(outputRomPathIsAFile)
			_spoilerLogPath = "./"; // outputRomPath points to a file, use cwd for the spoiler log
		else
			_spoilerLogPath = _outputRomPath; // outputRomPath points to a directory, use the same for the spoiler log
	}
	if(!Tools::endsWith(_spoilerLogPath, ".json") && *_spoilerLogPath.rbegin() != '/')
		_spoilerLogPath += "/";

	// Add the filename afterwards
	if(*_outputRomPath.rbegin() == '/')
		_outputRomPath += this->getHashSentence() + ".md";
	if(*_spoilerLogPath.rbegin() == '/')
		_spoilerLogPath += this->getHashSentence() + ".json";
}

SpawnLocation RandomizerOptions::getSpawnLocation() const
{
	if(_spawnLocation == SpawnLocation::RANDOM)
	{
		std::mt19937 rng(_seed);
		std::vector<SpawnLocation> spawnLocs = getAllSpawnLocations();
		Tools::shuffle(spawnLocs, rng);
		return *spawnLocs.begin();
	}

	return _spawnLocation; 
}

std::vector<std::string> RandomizerOptions::getHashWords() const
{
	std::vector<std::string> words = {
		"EkeEke", "Nail", "Horn", "Fang", "Magic", "Ice", "Thunder", "Gaia", "Fireproof", "Iron", "Spikes", "Healing", "Snow",
		"Mars", "Moon", "Saturn", "Venus", "Detox", "Statue", "Golden", "Mind", "Repair", "Casino", "Ticket", "Axe", "Ribbon",
		"Card", "Lantern", "Garlic", "Paralyze", "Chicken", "Death", "Jypta", "Sun", "Armlet", "Einstein", "Whistle", "Spell",
		"Book", "Lithograph", "Red", "Purple", "Jewel", "Pawn", "Ticket", "Gola", "Nole", "King", "Dragon", "Dahl", "Restoration",
		"Logs", "Oracle", "Stone", "Idol", "Key", "Safety", "Pass", "Bell", "Short", "Cake", "Life", "Stock", "Zak", "Duke",
		"Massan", "Gumi", "Ryuma", "Mercator", "Verla", "Destel", "Kazalt", "Greedly", "Dex", "Slasher", "Marley", "Nigel", "Friday",
		"Mir", "Miro", "Prospero", "Fara", "Orc", "Mushroom", "Slime", "Cyclops", "Ninja", "Ghost", "Tibor", "Knight", "Pockets",
		"Kado", "Kan", "Well", "Dungeon", "Loria", "Kayla", "Wally", "Ink", "Arthur", "Crypt", "Mummy", "Poison", "Labyrinth",
		"Palace", "Gold", "Waterfall", "Shrine", "Swamp", "Hideout", "Greenmaze", "Mines", "Lake", "Volcano", "Crate", "Jar", 
		"Helga", "Fahl", "Yard", "Twinkle", "Firedemon", "Spinner", "Golem", "Boulder"
	};

	std::mt19937 rng(_seed);
	Tools::shuffle(words, rng);
	return std::vector<std::string>(words.begin(), words.begin() + 4);
}

std::string RandomizerOptions::getPermalink() const
{
	std::ostringstream permalinkBuilder;
	permalinkBuilder 	<< MAJOR_RELEASE << ";"
						<< _seed << ";"
						<< _jewelCount << ";"
						<< (std::to_string(_fillingRate)) << ";"
						<< (_armorUpgrades ? "t" : "") << ";"
						<< (_shuffleTiborTrees ? "t" : "") << ";"
						<< (_saveAnywhereBook ? "t" : "") << ";"
						<< (spawnLocationToNumber(_spawnLocation)) << ";"
						<< (_dungeonSignHints ? "t" : "") << ";"
						<< (_allowSpoilerLog ? "t" : "");
	
	return "L" + base64_encode(permalinkBuilder.str()) + "S";
}

void RandomizerOptions::print(std::ostream& stream) const
{
	constexpr const char* trueStr = "enabled";
	constexpr const char* falseStr = "disabled";

	stream << "Seed: " << _seed << "\n";
	stream << "Permalink: " << this->getPermalink() << "\n";
	stream << "Hash: " << this->getHashSentence() << "\n\n";

	stream << "Starting location: ";
	if(_spawnLocation == SpawnLocation::RANDOM)
		stream << "Random (" << spawnLocationToString(this->getSpawnLocation()) << ")" << "\n";
	else
		stream << spawnLocationToString(this->getSpawnLocation()) << "\n";

	stream << "Jewel Count: " << _jewelCount << "\n";
	stream << "Armor upgrades: " << (_armorUpgrades ? trueStr : falseStr) << "\n";
	stream << "Filling rate: " << _fillingRate << "\n";
	stream << "Randomized Tibor trees: " << (_shuffleTiborTrees ? trueStr : falseStr) << "\n";
	stream << "Record Book: " << (_saveAnywhereBook ? trueStr : falseStr) << "\n";
	stream << "Fill dungeon signs with hints: " << (_dungeonSignHints ? trueStr : falseStr) << "\n";

	stream << "\n";
}

void RandomizerOptions::printPersonalSettings(std::ostream& stream) const
{
	constexpr const char* trueStr = "enabled";
	constexpr const char* falseStr = "disabled";

	stream << "Input ROM path: " <<  _inputRomPath << "\n";
	stream << "Output ROM path: " <<  _outputRomPath << "\n";
	stream << "Spoiler log path: " <<  _spoilerLogPath << "\n";
	stream << "Add ingame item tracker: " << (_addIngameItemTracker ? trueStr : falseStr) << "\n";
	stream << "HUD Color: " <<  _hudColor << "\n";

	stream << "\n";
}

