#pragma once

#include <string>
#include <vector>
#include <cstdint>

class GameText
{
public: 
	GameText();
	GameText(const std::string& text);
	
	bool isEmpty() const { return _bytes.empty(); }

	const std::string& getText() const { return _initialText; }
	void setText(const std::string& text);

	void addCharacter(const std::string& text, size_t i);
	void addCharacter(char character);

	const std::vector<uint8_t>& getBytes() const { return _bytes; }

	static uint8_t getCharacterByte(char character);
	static uint8_t getCharacterWidth(char character);

private:
	std::string _initialText;
	std::vector<uint8_t> _bytes;
	uint16_t _currentLineLength;
	uint8_t _currentLineCount;
};