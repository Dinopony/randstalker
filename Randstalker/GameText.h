#pragma once

#include <string>
#include <vector>
#include <cstdint>

class GameText
{
public:
	GameText();
	GameText(const std::string& text);

	const std::string& getText() const { return _initialText; }
	void setText(const std::string& text);

	void addCharacter(const std::string& text, size_t i);
	void addCharacter(char character);

	const std::string& getOutput() const { return _outputText; }

	static uint8_t getCharacterWidth(char character);

private:
	std::string _initialText;
	std::string _outputText;
	//	std::vector<uint8_t> _bytes;
	uint16_t _currentLineLength;
	uint8_t _currentLineCount;
};