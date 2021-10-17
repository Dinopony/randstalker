#pragma once

#include <fstream>
#include <sstream>
#include <set>
#include "World.hpp"

class SpoilerLog : public nlohmann::json
{
public:
	SpoilerLog(const RandomizerOptions& options, const World& world) : 
		nlohmann::json(),
		_options(options),
		_world(world)
	{
		(*this)["settings"] = _options.toJSON();
		(*this)["settings"]["seed"] = _options.getSeed();
		(*this)["settings"]["permalink"] = _options.getPermalink();
		(*this)["settings"]["hashSentence"] = _options.getHashSentence();

		(*this)["darkRegion"] = _world.darkenedRegion->getName();
		
		this->writeHints();
		this->writeTiborTrees();
		this->writeItems();
	}

	void writeHints()
	{
		(*this)["hints"]["Fortune Teller"] = _world.fortuneTellerHint;
		(*this)["hints"]["Oracle Stone"] = _world.oracleStoneHint;
		(*this)["hints"]["Lithograph"] = _world.jewelHints;
		(*this)["hints"]["King Nole's Cave sign"] = _world.whereIsLithographHint;		
		
		for (HintSign* sign : _world.hintSigns)
			(*this)["hints"][sign->getDescription()] = stripSpecialSymbolsFromString(_world.textLines.at(sign->getTextID()));
	}

	void writeTiborTrees()
	{
		if (_options.shuffleTiborTrees())
		{
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

			(*this)["tiborTrees"] = {
				pairs[0].first->getName() + " <===> " + pairs[0].second->getName(),
				pairs[1].first->getName() + " <===> " + pairs[1].second->getName(),
				pairs[2].first->getName() + " <===> " + pairs[2].second->getName(),
				pairs[3].first->getName() + " <===> " + pairs[3].second->getName(),
				pairs[4].first->getName() + " <===> " + pairs[4].second->getName(),
			};
		}
	}

	void writeItems()
	{
		for (auto& [key, region] : _world.regions)
		{
			std::vector<ItemSource*> sources = region->getItemSources();
			for(ItemSource* source : sources)
				(*this)["items"][region->getName()][source->getName()] = source->getItem()->getName();
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