#pragma once

#include <string>
#include <utility>
#include <vector>
#include "tools.hpp"

class Bitfield 
{
public:
    Bitfield() : _values(), _size(0)
    {
        _values.reserve(4);
    }

    Bitfield(std::vector<uint8_t> values, uint8_t size) : _values(std::move(values)), _size(size)
    {}

    void add(bool bit)
    {
        if (_size % 8 == 0)
        {
            _values.emplace_back((bit) ? 1 : 0);
        }
        else
        {
            uint8_t& value = _values[_values.size()-1];
            value <<= 1;
            if (bit)
                value += 1;
        }
        ++_size;
    }

    [[nodiscard]] bool get(uint32_t bit_id) const
    {
        size_t byteID = bit_id / 8;
        uint8_t byte = _values[byteID];

        if (byteID == _values.size()-1 && (_size % 8) != 0)
        {
            uint8_t lastByteSize = _size % 8;
            return (byte >> ((lastByteSize - 1) - (bit_id % 8))) & 0x1;
        }

        return (byte >> (7 - (bit_id % 8))) & 0x1;
    }

    [[nodiscard]] std::vector<uint8_t> to_bytes() const
    {
        return _values;
    }

    [[nodiscard]] uint32_t size() const
    {
        return _size;
    }

    Bitfield operator+(char bit) const
    {
        Bitfield newBF(*this);
        newBF.add(bit == '1');
        return newBF;
    }

    Bitfield operator+(const Bitfield& other) const
    {
        Bitfield newBF(*this);
        for (uint32_t i = 0; i < other._size; ++i)
            newBF.add(other.get(i));
        return newBF;
    }

    Bitfield& operator+=(const Bitfield& other)
    {
        for (uint32_t i = 0; i < other._size; ++i)
            this->add(other.get(i));
        return *this;
    }

    bool operator<(const Bitfield& other) const
    {
        if (_size == other._size)
        {
            for (size_t i = 0; i < _values.size(); ++i)
            {
                if (_values[i] < other._values[i])
                {
                    return true;
                }
                else if (_values[i] > other._values[i])
                {
                    return false;
                }
            }
        }
        else if (_size < other._size)
            return true;
        return false;
    }

    bool operator==(const Bitfield& other) const
    {
        if (_size != other._size)
            return false;
        for (size_t i = 0; i < _values.size(); ++i)
            if (_values[i] != other._values[i])
                return false;
        return true;
    }

    [[nodiscard]] std::string to_string() const
    {
        std::ostringstream oss;
        oss << "[";
        for (uint32_t i = 0; i < _size; ++i)
            oss << (this->get(i) ? "1" : "0");
        oss << "]";
        return oss.str();
    }

private:
    std::vector<uint8_t> _values;
    uint8_t _size;
};