#pragma once

#include <string>
#include <fstream>

constexpr auto ROM_SIZE = 2097152;

class GameROM 
{
public:
	GameROM(const std::string& inputPath) : _wasOpen(false)
	{
		_byteArray = new char[ROM_SIZE];

		std::ifstream file(inputPath, std::ios::binary);
		if (file.is_open())
		{
			file.read(_byteArray, ROM_SIZE);
			file.close();
			_wasOpen = true;
		}
	}

	GameROM(const GameROM& otherROM)
	{
		_wasOpen = otherROM._wasOpen;

		_byteArray = new char[ROM_SIZE];
		for (uint32_t i = 0; i < ROM_SIZE; ++i)
			_byteArray[i] = otherROM._byteArray[i];
	}

	~GameROM()
	{
		delete[] _byteArray;
	}

	bool isValid() const { return _wasOpen; }

	void setByte(uint32_t address, uint8_t byte)
	{
		if(address >= ROM_SIZE)
			return;

		_byteArray[address] = byte;
	}

	void setWord(uint32_t address, uint16_t word)
	{
		if(address >= ROM_SIZE-1)
			return;

		this->setByte(address, (word >> 8));
		this->setByte(address+1, (word & 0xFF));
	}

	void setLong(uint32_t address, uint32_t longWord)
	{
		if (address >= ROM_SIZE - 3)
			return;

		this->setWord(address, (longWord >> 16));
		this->setWord(address+2, (longWord & 0xFFFF));
	}

	void saveAs(const std::string& outputPath)
	{
		std::ofstream file(outputPath, std::ios::binary);
		file.write(_byteArray, ROM_SIZE);
		file.close();
	}

private:
	bool _wasOpen;
	char* _byteArray;
};