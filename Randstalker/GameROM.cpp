#include "GameROM.h"
#include "AsmCode.h"
#include <fstream>

constexpr uint32_t ROM_SIZE = 2097152;
constexpr uint32_t CODE_INJECTION_SECTOR_START_ADDRESS = 0x1FFAD0;


GameROM::GameROM(const std::string& inputPath) :
	_wasOpen(false),
	_currentCodeInjectionAddress(CODE_INJECTION_SECTOR_START_ADDRESS),
	_currentDataInjectionAddress(ROM_SIZE)
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

GameROM::GameROM(const GameROM& otherROM)
{
	_wasOpen = otherROM._wasOpen;

	_currentCodeInjectionAddress = otherROM._currentCodeInjectionAddress;
	_currentDataInjectionAddress = otherROM._currentDataInjectionAddress;

	_byteArray = new char[ROM_SIZE];
	for (uint32_t i = 0; i < ROM_SIZE; ++i)
		_byteArray[i] = otherROM._byteArray[i];
}

GameROM::~GameROM()
{
	delete[] _byteArray;
}

void GameROM::setByte(uint32_t address, uint8_t byte)
{
	if (address >= ROM_SIZE)
		return;

	_byteArray[address] = byte;
}

void GameROM::setWord(uint32_t address, uint16_t word)
{
	if (address >= ROM_SIZE - 1)
		return;

	this->setByte(address, (word >> 8));
	this->setByte(address + 1, (word & 0xFF));
}

void GameROM::setLong(uint32_t address, uint32_t longWord)
{
	if (address >= ROM_SIZE - 3)
		return;

	this->setWord(address, (longWord >> 16));
	this->setWord(address + 2, (longWord & 0xFFFF));
}

void GameROM::setBytes(uint32_t address, std::vector<uint8_t> bytes)
{
	for (uint32_t i = 0; i < bytes.size(); ++i)
	{
		if (address + i >= ROM_SIZE)
			return;

		_byteArray[address + i] = bytes[i];
	}
}

void GameROM::setCode(uint32_t address, const asm68k::Code& code)
{
	this->setBytes(address, code.getBytes());
}

uint32_t GameROM::injectByte(uint8_t byte)
{
	uint32_t injectionAddressOnStart = _currentCodeInjectionAddress;
	this->setByte(_currentCodeInjectionAddress, byte);
	_currentCodeInjectionAddress += 0x01;
	return injectionAddressOnStart;
}

uint32_t GameROM::injectWord(uint16_t word)
{
	uint32_t injectionAddressOnStart = _currentCodeInjectionAddress;
	this->setWord(_currentCodeInjectionAddress, word);
	_currentCodeInjectionAddress += 0x02;
	return injectionAddressOnStart;
}

uint32_t GameROM::injectLong(uint32_t longWord)
{
	uint32_t injectionAddressOnStart = _currentCodeInjectionAddress;
	this->setLong(_currentCodeInjectionAddress, longWord);
	_currentCodeInjectionAddress += 0x04;
	return injectionAddressOnStart;
}

uint32_t GameROM::injectCode(const asm68k::Code& code, const std::string& label)
{
	uint32_t injectionAddressOnStart = _currentCodeInjectionAddress;

	const std::vector<uint8_t>& bytes = code.getBytes();
	this->setBytes(_currentCodeInjectionAddress, bytes);
	_currentCodeInjectionAddress += static_cast<uint32_t>(bytes.size());
	
	if (!label.empty())
		this->storeAddress(label, injectionAddressOnStart);
	return injectionAddressOnStart;
}

uint32_t GameROM::injectDataBlock(std::vector<uint8_t> bytes, const std::string& name)
{
	this->reserveDataBlock((uint16_t)bytes.size(), name);
	this->setBytes(_currentDataInjectionAddress, bytes);
	return _currentDataInjectionAddress;
}

uint32_t GameROM::reserveDataBlock(uint16_t byteCount, const std::string& name)
{
	_currentDataInjectionAddress -= byteCount;
	this->storeAddress(name, _currentDataInjectionAddress);
	return _currentDataInjectionAddress;
}

void GameROM::saveAs(const std::string& outputPath)
{
	this->updateChecksum();

	std::ofstream file(outputPath, std::ios::binary);
	file.write(_byteArray, ROM_SIZE);
	file.close();
}

void GameROM::updateChecksum()
{
	uint16_t checksum = 0;
	for (uint32_t addr = 0x200; addr < ROM_SIZE; addr += 0x02)
	{
		uint8_t msb = _byteArray[addr];
		uint8_t lsb = _byteArray[addr + 1];
		uint16_t word = (uint16_t)(msb << 8) | lsb;
		checksum += word;
	}

	this->setWord(0x18E, checksum);
}
