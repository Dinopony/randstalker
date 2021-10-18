#pragma once

#include <string>

class WorldRegion;

class HintSign 
{
public:
	HintSign(uint16_t textID, const std::string& signDescription, WorldRegion* region) :
		_textIDs({ textID }),
		_description(signDescription),
		_contents(),
		_region(region)
	{}

	HintSign(const std::vector<uint16_t>& textIDs, const std::string& signDescription, WorldRegion* region) :
		_textIDs(textIDs),
		_description(signDescription),
		_contents(),
		_region(region)
	{}

	std::vector<uint16_t> getTextIDs() const { return _textIDs; }
	const std::string& getDescription() const { return _description; }
	const std::string& getText() const { return _contents; }
	WorldRegion* getRegion() const { return _region; }

	void setText(const std::string& contents) { _contents = contents; }

private:
	std::vector<uint16_t> _textIDs;
	std::string _description;
	std::string _contents;
	WorldRegion* _region;
};