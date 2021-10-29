#include "rom.hpp"
#include "code.hpp"
#include <fstream>
#include <iostream>

constexpr uint32_t ROM_SIZE = 2097152;

namespace md 
{
    ROM::ROM(const std::string& input_path) : _was_open(false)
    {
        _byte_array = new char[ROM_SIZE];

        std::ifstream file(input_path, std::ios::binary);
        if (file.is_open())
        {
            file.read(_byte_array, ROM_SIZE);
            file.close();
            _was_open = true;
        }
    }

    ROM::ROM(const ROM& other)
    {
        _was_open = other._was_open;
        _stored_addresses = other._stored_addresses;
        _empty_chunks = other._empty_chunks;

        _byte_array = new char[ROM_SIZE];
        for (uint32_t i = 0; i < ROM_SIZE; ++i)
            _byte_array[i] = other._byte_array[i];
    }

    ROM::~ROM()
    {
        delete[] _byte_array;
    }

    void ROM::set_byte(uint32_t address, uint8_t byte)
    {
        if (address >= ROM_SIZE)
            return;

        _byte_array[address] = byte;
    }

    void ROM::set_word(uint32_t address, uint16_t word)
    {
        if (address >= ROM_SIZE - 1)
            return;

        this->set_byte(address, (word >> 8));
        this->set_byte(address + 1, (word & 0xFF));
    }

    void ROM::set_long(uint32_t address, uint32_t longWord)
    {
        if (address >= ROM_SIZE - 3)
            return;

        this->set_word(address, (longWord >> 16));
        this->set_word(address + 2, (longWord & 0xFFFF));
    }

    void ROM::set_bytes(uint32_t address, std::vector<uint8_t> bytes)
    {
        for (uint32_t i = 0; i < bytes.size(); ++i)
        {
            this->set_byte(address + i, bytes[i]);
        }
    }

     void ROM::set_bytes(uint32_t address, const unsigned char* bytes, size_t bytes_size)
    {
        for (uint32_t i = 0; i < bytes_size; ++i)
        {
            this->set_byte(address + i, bytes[i]);
        }
    }

    void ROM::set_code(uint32_t address, const Code& code)
    {
        this->set_bytes(address, code.get_bytes());
    }

    uint32_t ROM::inject_bytes(const std::vector<uint8_t>& bytes, const std::string& label)
    {
        uint32_t size_to_inject = (uint32_t)bytes.size();
        uint32_t injection_addr = this->reserve_data_block(size_to_inject, label);
        this->set_bytes(injection_addr, bytes);
        return injection_addr;
    }

    uint32_t ROM::inject_bytes(const unsigned char* bytes, size_t size_to_inject, const std::string& label)
    {
        uint32_t injection_addr = this->reserve_data_block(static_cast<uint32_t>(size_to_inject), label);
        this->set_bytes(injection_addr, bytes, size_to_inject);
        return injection_addr;
    }

    uint32_t ROM::inject_code(const Code& code, const std::string& label)
    {
        return this->inject_bytes(code.get_bytes(), label);
    }

    uint32_t ROM::reserve_data_block(uint32_t byte_count, const std::string& label)
    {
        for(auto& pair : _empty_chunks)
        {
            if(pair.first >= pair.second)
                continue;

            size_t chunk_size = (pair.second - pair.first);
            if(chunk_size < byte_count)
                continue;

            uint32_t injection_addr = pair.first;
            pair.first += byte_count;

            // Don't allow an empty chunk to begin with an odd address
            if(pair.first % 2 != 0)
                pair.first++;

            if (!label.empty())
                this->store_address(label, injection_addr);

            return injection_addr;
        }

        throw std::out_of_range("Not enough empty room inside the ROM to inject data");
    }

    void ROM::data_chunk(uint32_t begin, uint32_t end, std::vector<uint8_t>& output) const
    {
        for (uint32_t addr = begin; addr < end; ++addr)
            output.push_back(this->get_byte(addr));
    }

    void ROM::data_chunk(uint32_t begin, uint32_t end, std::vector<uint16_t>& output) const
    {
        for (uint32_t addr = begin; addr < end; addr += 0x2)
            output.push_back(this->get_word(addr));
    }

    void ROM::data_chunk(uint32_t begin, uint32_t end, std::vector<uint32_t>& output) const
    {
        for (uint32_t addr = begin; addr < end; addr += 0x4)
            output.push_back(this->get_long(addr));
    }

    void ROM::mark_empty_chunk(uint32_t begin, uint32_t end)
    {
        // Don't allow an empty chunk to begin with an odd address
        if(begin % 2 != 0)
            begin++;

        for(auto& pair : _empty_chunks)
        {
            if((begin >= pair.first && begin < pair.second) || (end >= pair.first && end < pair.second))
            {
                std::cerr << "There is an overlap between empty chunks" << std::endl;
                return;
            }
        }

        for(uint32_t addr=begin ; addr < end ; ++addr)
            this->set_byte(addr, 0xFF);

        _empty_chunks.push_back(std::make_pair(begin, end));
    }

    uint32_t ROM::remaining_empty_bytes() const
    {
        uint32_t count = 0;
        for(auto& pair : _empty_chunks)
            count += (pair.second - pair.first);
        return count;
    }

    void ROM::save_as(const std::string& outputPath)
    {
        this->update_checksum();

        std::ofstream file(outputPath, std::ios::binary);
        file.write(_byte_array, ROM_SIZE);
        file.close();
    }

    void ROM::update_checksum()
    {
        uint16_t checksum = 0;
        for (uint32_t addr = 0x200; addr < ROM_SIZE; addr += 0x02)
        {
            uint8_t msb = _byte_array[addr];
            uint8_t lsb = _byte_array[addr + 1];
            uint16_t word = (uint16_t)(msb << 8) | lsb;
            checksum += word;
        }

        this->set_word(0x18E, checksum);
    }

}