#pragma once

#include <string>
#include <vector>
#include <map>

class AsmCode;

class GameROM 
{
public:
	GameROM(const std::string& inputPath);
	GameROM(const GameROM& otherROM);
	~GameROM();

	bool isValid() const { return _wasOpen; }

	uint8_t getByte(uint32_t address) const { return _byteArray[address]; }

	void setByte(uint32_t address, uint8_t byte);
	void setWord(uint32_t address, uint16_t word);
	void setLong(uint32_t address, uint32_t longWord);
	void setBytes(uint32_t address, std::vector<uint8_t> bytes);
	void setCode(uint32_t address, AsmCode& code);

	uint32_t injectByte(uint8_t byte);
	uint32_t injectWord(uint16_t word);
	uint32_t injectLong(uint32_t longWord);
	uint32_t injectCode(AsmCode& code, const std::string& label="");

	uint32_t injectDataBlock(std::vector<uint8_t> bytes, const std::string& name);
	uint32_t reserveDataBlock(uint16_t byteCount, const std::string& name);

	void storeAddress(const std::string& name, uint32_t address) { _storedAddresses[name] = address; }
	uint32_t getStoredAddress(const std::string& name) { return _storedAddresses.at(name); }

	uint32_t getCurrentInjectionAddress() { return _currentCodeInjectionAddress; }
	uint32_t getCurrentCodeInjectionAddress() { return _currentCodeInjectionAddress; }

	void saveAs(const std::string& outputPath);

private:
	void updateChecksum();

	bool _wasOpen;
	char* _byteArray;
	uint32_t _currentCodeInjectionAddress;
	uint32_t _currentDataInjectionAddress;
	std::map<std::string, uint32_t> _storedAddresses;
};
