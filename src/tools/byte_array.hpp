#pragma once

#include <vector>
#include <cstdint>

class ByteArray : public std::vector<uint8_t>
{
public:
    ByteArray() = default;

    void add_byte(uint8_t byte)
    { 
        this->emplace_back(byte);
    }
    
    void add_word(uint16_t word)
    {
        uint8_t msb = word >> 8;
        uint8_t lsb = word & 0x00FF;
        this->add_byte(msb);
        this->add_byte(lsb);
    }

    void add_long(uint32_t long_word)
    {
        uint16_t msw = long_word >> 16;
        uint16_t lsw = long_word & 0x0000FFFF;
        this->add_word(msw);
        this->add_word(lsw);
    }

    void add_bytes(const std::vector<uint8_t>& bytes)
    {
        this->insert(this->end(), bytes.begin(), bytes.end());
    }
};