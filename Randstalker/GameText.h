#pragma once

#include <string>
#include <vector>
#include <cstdint>

class GameText 
{
public: 
	GameText();
	GameText(const std::string& text);

	void addCharacter(char character);

	const std::vector<uint8_t>& getBytes() const { return _bytes; }

	static uint8_t getCharacterByte(char character);
	static uint8_t getCharacterWidth(char character);

private:
	std::vector<uint8_t> _bytes;
	uint16_t _currentLineLength;
};