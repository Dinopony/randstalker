#pragma once

#include <fstream>
#include <sstream>
#include <set>
#include "World.hpp"

class SpoilerLog : private std::ofstream 
{
public:
	SpoilerLog(const RandomizerOptions& options, const World& world) : 
		std::ofstream(),
		_options(options),
		_world(world)
	{}

	bool writeToFile()
	{
		if (!_options.allowSpoilerLog() || _options.getSpoilerLogPath().empty())
			return false;

		this->open(_options.getSpoilerLogPath());
		if (!this->is_open())
			return false;

		this->writeOptions();

		*this << "Dark region: " << _world.darkenedRegion->getName() << "\n";
		this->writeHints();
		this->writeTiborTrees();
		this->writeItems();

		this->close();
		return true;
	}

	void writeTitle(const std::string& title)
	{
		*this << "\n===================================\n";
		*this << "\t\t" << title;
		*this << "\n===================================\n\n";
	}

	void writeOptions()
	{
		this->writeTitle("Options");

		*this << "Seed: " << _options.getSeed() << "\n";
		*this << "Permalink: " << _options.getPermalink() << "\n";
		*this << "Hash: " << _options.getHashSentence() << "\n\n";

		*this << "Jewel count: " << std::to_string(_options.getJewelCount()) << "\n";
		*this << "Filling rate: " << std::to_string(_options.getFillingRate()) << "\n";
		*this << "Armor upgrades: " << (_options.useArmorUpgrades() ? "enabled" : "disabled") << "\n";
		*this << "Randomized Tibor trees: " << (_options.shuffleTiborTrees() ? "enabled" : "disabled") << "\n";
		*this << "Record Book: " << (_options.useRecordBook() ? "enabled" : "disabled") << "\n";
		*this << "In-game item tracker: " << (_options.addIngameItemTracker() ? "enabled" : "disabled") << "\n";

		if(_options.getSpawnLocation() == SpawnLocation::RANDOM)
			*this << "Starting location: Random (" << spawnLocationToString(_world.spawnLocation) << ")\n";
		else
			*this << "Starting location: " << spawnLocationToString(_options.getSpawnLocation()) << "\n";
			
		*this << "Fill dungeon signs with hints: " << (_options.fillDungeonSignsWithHints() ? "enabled" : "disabled") << "\n";

		*this << "\n";
	}

	void writeHints()
	{
		this->writeTitle("Hints");

		*this << "- Fortune Teller: \"" << _world.fortuneTellerHint << "\"\n";
		*this << "- Oracle Stone: \"" << _world.oracleStoneHint << "\"\n";
		
		*this << "- Lithograph: \"";
		for(const std::string& jewelHint : _world.jewelHints)
			*this << jewelHint << " ";
		*this << "\"\n";
		
		*this << "- King Nole's Cave sign: \"" << _world.whereIsLithographHint << "\"\n\n";

		for (HintSign* sign : _world.hintSigns)
			*this << "- " << sign->getDescription() << ": \"" << stripSpecialSymbolsFromString(_world.textLines.at(sign->getTextID())) << "\"\n";
	}

	void writeTiborTrees()
	{
		if (_options.shuffleTiborTrees())
		{
			this->writeTitle("Tibor Trees");

			std::pair<const TreeMap*, const TreeMap*> pairs[5];
			for (const TreeMap& map : _world.treeMaps)
			{
				uint16_t treeMapID = map.getTree();
				if (treeMapID == 0x0200)		pairs[0].first = &map;
				else if (treeMapID == 0x0216)	pairs[0].second = &map;
				else if (treeMapID == 0x021B)	pairs[1].first = &map;
				else if (treeMapID == 0x0219)	pairs[1].second = &map;
				else if (treeMapID == 0x0218)	pairs[2].first = &map;
				else if (treeMapID == 0x0201)	pairs[2].second = &map;
				else if (treeMapID == 0x021A)	pairs[3].first = &map;
				else if (treeMapID == 0x0217)	pairs[3].second = &map;
				else if (treeMapID == 0x01FF)	pairs[4].first = &map;
				else if (treeMapID == 0x01FE)	pairs[4].second = &map;
			}

			for (int i = 0; i < 5; ++i)
				*this << "- " << pairs[i].first->getName() << " <===> " << pairs[i].second->getName() << "\n";
		}
	}

	void writeItems()
	{
		this->writeTitle("Items");

		for (auto& [key, region] : _world.regions)
		{
			std::vector<ItemSource*> sources = region->getItemSources();
			if (sources.empty())
				continue;

			*this << region->getName() << "\n";

			for (ItemSource* source : sources)
				if (source->getItem())
					*this << "   |_ \"" << source->getName() << "\" : [" << source->getItem()->getName() << "]\n";
			*this << "\n";
		}
	}

private:
	static std::string stripSpecialSymbolsFromString(const std::string& str)
	{
		const std::set<char> strippedChars = { 
			'\x10', '\x11', '\x12', '\x13', '\x02', '\x18', '\x19', 
			'\x1A', '\x1B', '\x1C', '\x03', '\x1D', '\x1E', '\x1F',
		};

		std::ostringstream oss;
		for (size_t i = 0; i < str.length(); ++i)
		{
			if (str[i] == '\n')
				oss << ' ';
			else if (!strippedChars.count(str[i]))
				oss << str[i];
		}

		return oss.str();
	}

	const RandomizerOptions& _options;
	const World& _world;
};