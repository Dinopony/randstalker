#pragma once

#include <string>
#include <vector>
#include <cstdint>

class GameText
{
public:
    GameText(uint8_t lines_in_textbox = 3);
    GameText(const std::string& text, uint8_t lines_in_textbox = 3);

    const std::string& getText() const { return _initialText; }
    void setText(const std::string& text);

    void addCharacter(const std::string& text, size_t i);
    void addCharacter(char character);

    const std::string& getOutput() const { return _outputText; }

    static uint8_t getCharacterWidth(char character);

private:
    std::string _initialText;
    std::string _outputText;
    uint16_t _currentLineLength;
    uint8_t _currentLineCount;
    uint8_t _linesInTextbox;
};