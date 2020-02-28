#include "GameText.h"
#include <sstream>

GameText::GameText() : _currentLineLength(0), _currentLineCount(0)
{}

GameText::GameText(const std::string& text) : _currentLineLength(0), _currentLineCount(0)
{
	this->setText(text);
}

void GameText::setText(const std::string& text)
{
	_bytes.clear();

	if (!text.empty())
	{
		for (size_t i = 0; i < text.size(); ++i)
			this->addCharacter(text, i);
		this->addCharacter('\0');
	}
}

void GameText::addCharacter(const std::string& text, size_t i)
{
	char character = text[i];

	// If we start a new word, check if we can finish it on the same line
	if (!_bytes.empty() && character != ' ' && (*_bytes.rbegin()) == 0x00)
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
	if (character == '\0')
	{
		this->addCharacter('\t');
		_bytes.push_back(0xFF);
		_bytes.push_back(0xFF);
	}
	else if (character == '\t')
	{
		_bytes.push_back(0xFF);
		_bytes.push_back(0xF0);
		_currentLineCount = 0;
	}
	else if (character == '\n')
	{
		if (++_currentLineCount == 3)
			this->addCharacter('\t'); // Add a 'wait for input' character if textbox is going to scroll
		_bytes.push_back(0x00);
		_bytes.push_back(0x6C);
		_currentLineLength = 0;
	}
	else
	{
		if (_currentLineLength >= 0x105)
		{
			uint8_t previousByte = *_bytes.rbegin();
			bool needsHyphen = (previousByte >= 0x0B && previousByte <= 0x3E);
			this->addCharacter('\n');
			if (needsHyphen)
				this->addCharacter('-');
		}

		_bytes.push_back(0x00);
		_bytes.push_back(getCharacterByte(character));
		_currentLineLength += getCharacterWidth(character);
	}
}

uint8_t GameText::getCharacterByte(char character)
{
	// 'a' - 'z' ===> 0x25 - 0x3E
	if (character >= 'a' && character <= 'z')
		return 0x25 + (character - 'a');
	// ' ' ===> 0x00
	else if (character == ' ')
		return 0x00;
	// 'A' - 'Z' ===> 0x0B - 0x24
	else if (character >= 'A' && character <= 'Z')
		return 0x0B + (character - 'A');
	// '0' - '9' ===> 0x01 - 0x0A
	else if(character >= '0' && character <= '9')
		return 0x01 + (character - '0');

	// Other characters
	switch (character)
	{
	case '*':	return 0x3F;
	case '.':	return 0x40;
	case ',':	return 0x41;
	case '?':	return 0x42;
	case '!':	return 0x43;
	case '/':	return 0x44;
	case '<':	return 0x45;
	case '>':	return 0x46;
	case ':':	return 0x47;
	case '-':	return 0x48;
	case '\'':	return 0x49;
	case '"':	return 0x4A;
	case '%':	return 0x4B;
	case '#':	return 0x4C;
	case '&':	return 0x4D;
	case '(':	return 0x4E;
	case ')':	return 0x4F;
	case '=':	return 0x50;
	default:	return 0x42; // '?'
	}
}

uint8_t GameText::getCharacterWidth(char character)
{
	switch (character)
	{
	case ' ':	return 8;
	case '0':	return 8;
		//		case '1':	return 8;
	case '2':	return 8;
		//		case '3':	return 8;
		//		case '4':	return 8;
		//		case '5':	return 8;
		//		case '6':	return 8;
		//		case '7':	return 8;
		//		case '8':	return 8;
		//		case '9':	return 8;
	case 'A':	return 8;
	case 'B':	return 8;
		//		case 'C':	return 8;
	case 'D':	return 8;
	case 'E':	return 8;
	case 'F':	return 8;
		//		case 'G':	return 8;
		//		case 'H':	return 8;
	case 'I':	return 5;
		//		case 'J':	return 8;
		//		case 'K':	return 8;
		//		case 'L':	return 8;
		//		case 'M':	return 8;
		//		case 'N':	return 8;
	case 'O':	return 8;
		//		case 'P':	return 8;
		//		case 'Q':	return 8;
		//		case 'R':	return 8;
		//		case 'S':	return 8;
	case 'T':	return 9;
		//		case 'U':	return 8;
		//		case 'V':	return 8;
		//		case 'W':	return 8;
		//		case 'X':	return 8;
	case 'Y':	return 8;
		//		case 'Z':	return 8;
	case 'a':	return 9;
	case 'b':	return 8;
	case 'c':	return 8;
	case 'd':	return 8;
	case 'e':	return 8;
	case 'f':	return 7;
	case 'g':	return 8;
	case 'h':	return 8;
	case 'i':	return 5;
	case 'j':	return 6;
	case 'k':	return 8;
	case 'l':	return 6;
	case 'm':	return 10;
	case 'n':	return 8;
	case 'o':	return 8;
	case 'p':	return 8;
		//		case 'q':	return 8;
	case 'r':	return 8;
	case 's':	return 8;
	case 't':	return 8;
	case 'u':	return 8;
	case 'v':	return 9;
	case 'w':	return 11;
	case 'x':	return 8;
	case 'y':	return 8;
		//		case 'z':	return 8;
	case '*':	return 9;
	case '.':	return 5;
	case ',':	return 5;
	case '?':	return 8;
	case '!':	return 5;
		//		case '/':	return 8;
		//		case '<':	return 8;
		//		case '>':	return 8;
	case ':':	return 4;
		//		case '-':	return 8;
	case '\'':	return 4;
		//		case '"':	return 8;
		//		case '%':	return 8;
		//		case '#':	return 8;
		//		case '&':	return 8;
		//		case '(':	return 8;
		//		case ')':	return 8;
		//		case '=':	return 8;
	default:	return 8;
	}
}
