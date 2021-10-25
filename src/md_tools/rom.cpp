#include "rom.hpp"
#include "code.hpp"
#include <fstream>
#include <iostream>

constexpr uint32_t ROM_SIZE = 2097152;

namespace md 
{
	ROM::ROM(const std::string& inputPath) : _wasOpen(false)
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

	ROM::ROM(const ROM& otherROM)
	{
		_wasOpen = otherROM._wasOpen;
		_storedAddresses = otherROM._storedAddresses;
		_emptyChunks = otherROM._emptyChunks;

		_byteArray = new char[ROM_SIZE];
		for (uint32_t i = 0; i < ROM_SIZE; ++i)
			_byteArray[i] = otherROM._byteArray[i];
	}

	ROM::~ROM()
	{
		delete[] _byteArray;
	}

	void ROM::setByte(uint32_t address, uint8_t byte)
	{
		if (address >= ROM_SIZE)
			return;

		_byteArray[address] = byte;
	}

	void ROM::setWord(uint32_t address, uint16_t word)
	{
		if (address >= ROM_SIZE - 1)
			return;

		this->setByte(address, (word >> 8));
		this->setByte(address + 1, (word & 0xFF));
	}

	void ROM::setLong(uint32_t address, uint32_t longWord)
	{
		if (address >= ROM_SIZE - 3)
			return;

		this->setWord(address, (longWord >> 16));
		this->setWord(address + 2, (longWord & 0xFFFF));
	}

	void ROM::setBytes(uint32_t address, std::vector<uint8_t> bytes)
	{
		for (uint32_t i = 0; i < bytes.size(); ++i)
		{
			this->setByte(address + i, bytes[i]);
		}
	}

	void ROM::setCode(uint32_t address, const Code& code)
	{
		this->setBytes(address, code.getBytes());
	}

	uint32_t ROM::injectBytes(const std::vector<uint8_t>& bytes, const std::string& label)
	{
		uint32_t sizeToInject = (uint32_t)bytes.size();
		uint32_t injectionAddress = this->reserveDataBlock(sizeToInject, label);
		this->setBytes(injectionAddress, bytes);
		return injectionAddress;
	}

	uint32_t ROM::injectCode(const Code& code, const std::string& label)
	{
		return this->injectBytes(code.getBytes(), label);
	}

	uint32_t ROM::reserveDataBlock(uint32_t byteCount, const std::string& label)
	{
		for(auto& pair : _emptyChunks)
		{
			size_t chunkSize = (pair.second - pair.first) + 1;
			if(chunkSize < byteCount)
				continue;

			uint32_t injectionAddress = pair.first;
			pair.first += byteCount;

			// Don't allow an empty chunk to begin with an odd address
			if(pair.first % 2 != 0)
				pair.first++;

			if (!label.empty())
				this->storeAddress(label, injectionAddress);

			return injectionAddress;
		}

		throw std::out_of_range("Not enough empty room inside the ROM to inject data");
	}

	void ROM::getDataChunk(uint32_t begin, uint32_t end, std::vector<uint8_t>& output)
	{
		for (uint32_t addr = begin; addr < end; ++addr)
			output.push_back(this->getByte(addr));
	}

	void ROM::getDataChunk(uint32_t begin, uint32_t end, std::vector<uint16_t>& output)
	{
		for (uint32_t addr = begin; addr < end; addr += 0x2)
			output.push_back(this->getWord(addr));
	}

	void ROM::getDataChunk(uint32_t begin, uint32_t end, std::vector<uint32_t>& output)
	{
		for (uint32_t addr = begin; addr < end; addr += 0x4)
			output.push_back(this->getLong(addr));
	}

	void ROM::markChunkAsEmpty(uint32_t begin, uint32_t end)
	{
		// Don't allow an empty chunk to begin with an odd address
		if(begin % 2 != 0)
			begin++;

		for(auto& pair : _emptyChunks)
		{
			if((begin >= pair.first && begin < pair.second) || (end >= pair.first && end < pair.second))
			{
				std::cerr << "There is an overlap between empty chunks" << std::endl;
				return;
			}
		}

		for(uint32_t addr=begin ; addr < end ; ++addr)
			this->setByte(addr, 0xFF);

		_emptyChunks.push_back(std::make_pair(begin, end));
	}

	uint32_t ROM::countEmptyBytes() const
	{
		uint32_t count = 0;
		for(auto& pair : _emptyChunks)
			count += (pair.second - pair.first) + 1;
		return count;
	}

	void ROM::saveAs(const std::string& outputPath)
	{
		this->updateChecksum();

		std::ofstream file(outputPath, std::ios::binary);
		file.write(_byteArray, ROM_SIZE);
		file.close();
	}

	void ROM::updateChecksum()
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

}