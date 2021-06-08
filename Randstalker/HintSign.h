#pragma once

#include <string>

class WorldRegion;

class HintSign 
{
public:
	HintSign(uint16_t textID, const std::string& signDescription, WorldRegion* region) :
		_textID(textID),
		_description(signDescription),
		_region(region)
	{}

	uint16_t getTextID() const { return _textID; }
	const std::string& getDescription() const { return _description; }
	WorldRegion* getRegion() const { return _region; }

private:
	uint16_t _textID;
	std::string _description;
	WorldRegion* _region;
};