#include "game_text.hpp"
#include <sstream>

GameText::GameText(uint8_t lines_in_textbox) : 
    _currentLineLength   (0), 
    _currentLineCount    (0), 
    _linesInTextbox      (lines_in_textbox)
{}

GameText::GameText(const std::string& text, uint8_t lines_in_textbox) : 
    _currentLineLength   (0), 
    _currentLineCount    (0), 
    _linesInTextbox      (lines_in_textbox)
{
    this->setText(text);
}

void GameText::setText(const std::string& text)
{
    _initialText = text;
    _outputText = "";

    for (size_t i = 0; i < text.size(); ++i)
        this->addCharacter(text, i);

    _outputText += "\x03"; // EOL
}

void GameText::addCharacter(const std::string& text, size_t i)
{
    char character = text[i];

    // If we start a new word, check if we can finish it on the same line
    if (!_outputText.empty() && character != ' ' && (*_outputText.rbegin()) == ' ')
    {
        uint16_t wordWidth = 0;
        char currentWordChar = character;
        while (currentWordChar != '\n' && currentWordChar != ' ' && currentWordChar != '\0')
        {
            wordWidth += getCharacterWidth(currentWordChar);
            currentWordChar = text[++i];
        }

        // Word is too big to fit on the line, skip a line
        if (_currentLineLength + wordWidth >= 0x105)
            this->addCharacter('\n');
    }

    this->addCharacter(character);
}

void GameText::addCharacter(char character)
{
    if (character == '\n')
    {
        _currentLineLength = 0;
        if (*_outputText.rbegin() == ' ')
            _outputText = _outputText.substr(0, _outputText.size() - 1);

        if (++_currentLineCount == _linesInTextbox)
        {
            _outputText += "\x1E";
            _currentLineCount = 0;
        } 
        _outputText += "\xA";

        return;
    } 
    
    if (_currentLineLength >= 0x105)
    {
        this->addCharacter('\n');

        uint8_t previousByte = *_outputText.rbegin();
        bool needsHyphen = (previousByte >= 'A' && previousByte <= 'z');
        if (needsHyphen)
            this->addCharacter('-');
    }

    _outputText += character;
    _currentLineLength += getCharacterWidth(character);
}

uint8_t GameText::getCharacterWidth(char character)
{
    switch (character)
    {
    case ' ':    return 8;
    case '0':    return 8;
        //        case '1':    return 8;
    case '2':    return 8;
        //        case '3':    return 8;
        //        case '4':    return 8;
        //        case '5':    return 8;
        //        case '6':    return 8;
        //        case '7':    return 8;
        //        case '8':    return 8;
        //        case '9':    return 8;
    case 'A':    return 8;
    case 'B':    return 8;
        //        case 'C':    return 8;
    case 'D':    return 8;
    case 'E':    return 8;
    case 'F':    return 8;
        //        case 'G':    return 8;
        //        case 'H':    return 8;
    case 'I':    return 5;
        //        case 'J':    return 8;
        //        case 'K':    return 8;
        //        case 'L':    return 8;
        //        case 'M':    return 8;
        //        case 'N':    return 8;
    case 'O':    return 8;
        //        case 'P':    return 8;
        //        case 'Q':    return 8;
        //        case 'R':    return 8;
        //        case 'S':    return 8;
    case 'T':    return 9;
        //        case 'U':    return 8;
        //        case 'V':    return 8;
        //        case 'W':    return 8;
        //        case 'X':    return 8;
    case 'Y':    return 8;
        //        case 'Z':    return 8;
    case 'a':    return 9;
    case 'b':    return 8;
    case 'c':    return 8;
    case 'd':    return 8;
    case 'e':    return 8;
    case 'f':    return 7;
    case 'g':    return 8;
    case 'h':    return 8;
    case 'i':    return 5;
    case 'j':    return 6;
    case 'k':    return 8;
    case 'l':    return 6;
    case 'm':    return 10;
    case 'n':    return 8;
    case 'o':    return 8;
    case 'p':    return 8;
        //        case 'q':    return 8;
    case 'r':    return 8;
    case 's':    return 8;
    case 't':    return 8;
    case 'u':    return 8;
    case 'v':    return 9;
    case 'w':    return 11;
    case 'x':    return 8;
    case 'y':    return 8;
        //        case 'z':    return 8;
    case '*':    return 9;
    case '.':    return 5;
    case ',':    return 5;
    case '?':    return 8;
    case '!':    return 5;
        //        case '/':    return 8;
        //        case '<':    return 8;
        //        case '>':    return 8;
    case ':':    return 4;
        //        case '-':    return 8;
    case '\'':    return 4;
        //        case '"':    return 8;
        //        case '%':    return 8;
        //        case '#':    return 8;
        //        case '&':    return 8;
        //        case '(':    return 8;
        //        case ')':    return 8;
        //        case '=':    return 8;
    default:    return 8;
    }
}
