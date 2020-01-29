#pragma once

#include <string>
#include <fstream>

constexpr auto ROM_SIZE = 2097152;
constexpr auto CODE_INJECTION_SECTOR_START_ADDRESS = 0x1FFAD0;

class GameROM 
{
public:
	GameROM(const std::string& inputPath) : _wasOpen(false), _currentInjectionAddress(CODE_INJECTION_SECTOR_START_ADDRESS)
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

	uint32_t injectByte(uint8_t byte) 
	{
		uint32_t injectionAddressOnStart = _currentInjectionAddress;
		this->setByte(_currentInjectionAddress, byte);
		_currentInjectionAddress += 0x01;
		return injectionAddressOnStart;
	}

	uint32_t injectWord(uint16_t word) 
	{
		uint32_t injectionAddressOnStart = _currentInjectionAddress;
		this->setWord(_currentInjectionAddress, word);
		_currentInjectionAddress += 0x02;
		return injectionAddressOnStart;
	}

	uint32_t injectLong(uint32_t longWord) 
	{
		uint32_t injectionAddressOnStart = _currentInjectionAddress;
		this->setLong(_currentInjectionAddress, longWord);
		_currentInjectionAddress += 0x04;
		return injectionAddressOnStart;
	}

	uint32_t getCurrentInjectionAddress() { return _currentInjectionAddress; }

	void saveAs(const std::string& outputPath)
	{
		this->updateChecksum();

		std::ofstream file(outputPath, std::ios::binary);
		file.write(_byteArray, ROM_SIZE);
		file.close();
	}

private:
	void updateChecksum()
	{
		uint16_t checksum = 0;
		for(uint32_t addr = 0x200 ; addr < ROM_SIZE ; addr += 0x02 )
		{
			uint8_t msb = _byteArray[addr];
			uint8_t lsb = _byteArray[addr + 1];
			uint16_t word = (uint16_t)(msb << 8) | lsb;
			checksum += word;
		}

		this->setWord(0x18E, checksum);
	}

	bool _wasOpen;
	char* _byteArray;
	uint32_t _currentInjectionAddress;
};
