#pragma once

#include <fstream>
#include "World.h"

class SpoilerLog : private std::ofstream 
{
public:
	SpoilerLog(const RandomizerOptions& options, const World& world) : 
		std::ofstream(),
		_options(options),
		_world(world)
	{}

	void writeToFile()
	{
		if (_options.getSpoilerLogPath().empty())
			return;

		this->open(_options.getSpoilerLogPath());
		if (!this->is_open())
			return;

		this->writeOptions();

		*this << "Dark region: " << _world.darkenedRegion->getName() << "\n";
		this->writeHints();
		this->writeTiborTrees();
		this->writeItems();

		this->close();
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

		*this << "Seed: " << _options.getSeedString() << "\n";
		*this << "Armor upgrades: " << (_options.useArmorUpgrades() ? "enabled" : "disabled") << "\n";
		*this << "Randomized Tibor trees: " << (_options.shuffleTiborTrees() ? "enabled" : "disabled") << "\n";
		*this << "Record Book: " << (_options.useRecordBook() ? "enabled" : "disabled") << "\n";
		*this << "In-game item tracker: " << (_options.addIngameItemTracker() ? "enabled" : "disabled") << "\n";
		*this << "Starting location: " << _options.getSpawnLocationAsString() << "\n";
		*this << "Replace original game hints: " << (_options.replaceOriginalGameHints() ? "enabled" : "disabled") << "\n";

		*this << "\n";
	}

	void writeHints()
	{
		this->writeTitle("Hints");

		std::string lithographHint = _world.lithographHint.getText();
		lithographHint.erase(std::remove(lithographHint.begin(), lithographHint.end(), '\n'), lithographHint.end());
		*this << "- Lithograph: \"" << lithographHint << "\"\n";
		*this << "- Fortune Teller: \"" << _world.ingameTexts.at(0x27CE6).getText() << "\"\n";

		for (const auto& [addr, name] : _world.roadSigns)
			*this << "- " << name << ": \"" << _world.ingameTexts.at(addr).getText() << "\"\n";
		*this << "- King Nole's Cave sign: \"" << _world.ingameTexts.at(0x27958).getText() << "\"\n";
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
			std::vector<ItemSource*> sources = region->getAllItemSources();
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
	const RandomizerOptions& _options;
	const World& _world;
};