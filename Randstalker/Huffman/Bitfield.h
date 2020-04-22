#pragma once

#include <string>
#include <vector>
#include "Tools.h"

class Bitfield 
{
public:
	Bitfield() : _values(), _size(0)
	{}

//	Bitfield(const std::string& bits) : _bits(bits)
//	{}

	void add(bool bit)
	{
		if (_size % 8 == 0)
		{
			_values.push_back((bit) ? 1 : 0);
		}
		else
		{
			uint8_t& value = *_values.rbegin();
			value <<= 1;
			if (bit)
				value += 1;
		}
		++_size;
	}

	bool get(uint32_t bitID) const
	{
		size_t byteID = bitID / 8;
		uint8_t byte = _values[byteID];

		if (byteID == _values.size()-1 && (_size % 8) != 0)
		{
			uint8_t lastByteSize = _size % 8;
			return (byte >> ((lastByteSize - 1) - (bitID % 8))) & 0x1;
		}

		return (byte >> (7 - (bitID % 8))) & 0x1;
	}

	std::vector<uint8_t> toValues() const
	{
		return _values;
	}

	uint32_t size() const
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
		for (size_t i = 0; i < other._size; ++i)
			newBF.add(other.get(i));
		return newBF;
	}

	Bitfield& operator+=(const Bitfield& other)
	{
		*this = *this + other;
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

	std::string toString() const
	{
		std::ostringstream oss;
		oss << "[";
		for (size_t i = 0; i < _size; ++i)
			oss << (this->get(i) ? "1" : "0");
		oss << "]";
		return oss.str();
	}

private:
	std::vector<uint8_t> _values;
	uint8_t _size;
};