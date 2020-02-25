#pragma once

#include <cstdint>
#include <vector>

namespace md
{
    enum class Size { BYTE, WORD, LONG };

    class Param {
    public:
        Param() {}

        virtual uint16_t getXn() const = 0;
        virtual uint16_t getM() const = 0;

        uint16_t getMXn() const { return (this->getM() << 3) + this->getXn(); }
        uint16_t getXnM() const { return (this->getXn() << 3) + this->getM(); }

        virtual std::vector<uint8_t> getAdditionnalData() const { return {}; }
    };

    ////////////////////////////////////////////////////////////////////////////

    class Register : public Param {
    public:
        Register(uint8_t code) :
            _code(code)
        {}

        virtual uint16_t getXn() const { return _code & 0x7; }

        virtual bool isDataRegister() const = 0;
        bool isAddressRegister() const { return !isDataRegister(); }

    protected:
        uint8_t _code;
    };

    class AddressRegister : public Register {
    public:
        AddressRegister(uint8_t code) : Register(code) {}

        virtual uint16_t getM() const { return 0x1; }
        virtual bool isDataRegister() const { return false; }
    };

    class DataRegister : public Register {
    public:
        DataRegister(uint8_t code) : Register(code) {}

        virtual uint16_t getM() const { return 0x0; }
        virtual bool isDataRegister() const { return true; }
    };

    ////////////////////////////////////////////////////////////////////////////

    class DirectAddress : public Param {
    public:
        DirectAddress(uint32_t address) :
            _address(address)
        {}

        virtual uint16_t getM() const { return 0x7; }
        virtual uint16_t getXn() const { return 0x1; }

        uint32_t getAddress() const { return _address; }

        virtual std::vector<uint8_t> getAdditionnalData() const
        {
            std::vector<uint8_t> addressBytes;
            addressBytes.push_back((_address >> 24) & 0xFF);
            addressBytes.push_back((_address >> 16) & 0xFF);
            addressBytes.push_back((_address >> 8) & 0xFF);
            addressBytes.push_back(_address & 0xFF);
            return addressBytes;
        }

    private:
        uint32_t _address;
    };

    ////////////////////////////////////////////////////////////////////////////

    class AddressInRegister : public Param {
    public:
        AddressInRegister(const AddressRegister& reg, uint16_t offset = 0) :
            _offset(offset),
            _reg(reg)
        {}

        virtual uint16_t getM() const { return (_offset > 0) ? 0x5 : 0x2; }
        virtual uint16_t getXn() const { return _reg.getXn(); }

        virtual std::vector<uint8_t> getAdditionnalData() const
        {
            if (_offset > 0)
            {
                std::vector<uint8_t> offsetBytes;
                offsetBytes.push_back((_offset >> 8) & 0xFF);
                offsetBytes.push_back(_offset & 0xFF);
                return offsetBytes;
            }
            else return {};
        }

    private:
        AddressRegister _reg;
        uint16_t _offset;
    };

    ////////////////////////////////////////////////////////////////////////////

    class AddressWithIndex : public Param {
    public:
        AddressWithIndex(const AddressRegister& baseAddrReg, const Register& offsetReg, Size offsetRegSize, uint8_t additionnalOffset = 0) :
            _baseAddrReg(baseAddrReg),
            _offsetReg(offsetReg),
            _offsetRegSize(offsetRegSize),
            _additionnalOffset(additionnalOffset)
        {}

        virtual uint16_t getM() const { return 0x6; }
        virtual uint16_t getXn() const { return _baseAddrReg.getXn(); }

        virtual std::vector<uint8_t> getAdditionnalData() const
        {
            uint8_t msb = (_offsetReg.getMXn() & 0x0F) << 4;
            if (_offsetRegSize == Size::LONG)
                msb |= 0x8;

            uint8_t lsb = _additionnalOffset;

            std::vector<uint8_t> briefExtensionWord;
            briefExtensionWord.push_back(msb);
            briefExtensionWord.push_back(lsb);
            return briefExtensionWord;
        }

    private:
        const AddressRegister& _baseAddrReg;
        const Register& _offsetReg;
        Size _offsetRegSize;
        uint8_t _additionnalOffset;
    };

    ////////////////////////////////////////////////////////////////////////////

    class ImmediateValue : public Param {
    public:
        explicit ImmediateValue(uint8_t value) : _size(Size::BYTE), _value(value) {}
        explicit ImmediateValue(uint16_t value) : _size(Size::WORD), _value(value) {}
        explicit ImmediateValue(uint32_t value) : _size(Size::LONG), _value(value) {}

        virtual uint16_t getXn() const { return 0x4; }
        virtual uint16_t getM() const { return 0x7; }

        virtual std::vector<uint8_t> getAdditionnalData() const
        {
            std::vector<uint8_t> valueBytes;
            if (_size == Size::LONG)
            {
                valueBytes.push_back((_value >> 24) & 0xFF);
                valueBytes.push_back((_value >> 16) & 0xFF);
            }

            valueBytes.push_back((_value >> 8) & 0xFF);
            valueBytes.push_back(_value & 0xFF);

            return valueBytes;
        }

    private:
        uint32_t _value;
        Size _size;
    };
}

#define reg_A0 md::AddressRegister(0)
#define reg_A1 md::AddressRegister(1)
#define reg_A2 md::AddressRegister(2)
#define reg_A3 md::AddressRegister(3)
#define reg_A4 md::AddressRegister(4)
#define reg_A5 md::AddressRegister(5)
#define reg_A6 md::AddressRegister(6)
#define reg_A7 md::AddressRegister(7)

#define reg_D0 md::DataRegister(0)
#define reg_D1 md::DataRegister(1)
#define reg_D2 md::DataRegister(2)
#define reg_D3 md::DataRegister(3)
#define reg_D4 md::DataRegister(4)
#define reg_D5 md::DataRegister(5)
#define reg_D6 md::DataRegister(6)
#define reg_D7 md::DataRegister(7)

#define reg_D0_D7 reg_D0,reg_D1,reg_D2,reg_D3,reg_D4,reg_D5,reg_D6,reg_D7
#define reg_A0_A6 reg_A0,reg_A1,reg_A2,reg_A3,reg_A4,reg_A5,reg_A6

template<typename T>
inline constexpr md::ImmediateValue bval_(T value)
{
    return md::ImmediateValue(static_cast<uint8_t>(value));
}

template<typename T>
inline constexpr md::ImmediateValue wval_(T value)
{
    return md::ImmediateValue(static_cast<uint16_t>(value));
}

template<typename T>
inline constexpr md::ImmediateValue lval_(T value)
{
    return md::ImmediateValue(static_cast<uint32_t>(value));
}

template<typename T = void>
inline constexpr md::DirectAddress addr_(uint32_t address)
{
    return md::DirectAddress(address);
}

template<typename T = void>
inline constexpr md::AddressInRegister addr_(const md::AddressRegister& addressRegister)
{
    return md::AddressInRegister(addressRegister);
}

template<typename T>
inline constexpr md::AddressInRegister addr_(const md::AddressRegister& addressRegister, T offset)
{
    return md::AddressInRegister(addressRegister, offset);
}

template<typename T = void>
inline constexpr md::AddressWithIndex addr_(  const md::AddressRegister& addressRegister,
                                              const md::DataRegister& offsetDataRegister,
                                              md::Size dataRegisterSize = md::Size::LONG,
                                              uint8_t additionnalOffset = 0)
{
    return md::AddressWithIndex(addressRegister, offsetDataRegister, dataRegisterSize, additionnalOffset);
}
