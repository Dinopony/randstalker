#pragma once

#include <vector>
#include <map>
#include <string>

namespace asm68k
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

    class Data : public Param {
    public:
        Data(uint8_t value) : _size(Size::BYTE), _value(value) {}
        Data(uint16_t value) : _size(Size::WORD), _value(value) {}
        Data(uint32_t value) : _size(Size::LONG), _value(value) {}

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

//        operator uint8_t() const { return static_cast<uint8_t>(_value); }
//        operator uint16_t() const { return static_cast<uint16_t>(_value); }
//        operator uint32_t() const { return static_cast<uint32_t>(_value); }

    private:
        uint32_t _value;
        Size _size;
    };

    ////////////////////////////////////////////////////////////////////////////

    class Code {
    public:
        Code() {}

        void addByte(uint8_t byte);
        void addWord(uint16_t word);
        void addLong(uint32_t longword);
        void addBytes(const std::vector<uint8_t>& bytes);
        void addOpcode(uint16_t opcode);

        Code& jsr(uint32_t address);
        Code& jmp(uint32_t address);

        Code& cmpi(const Data& value, const Param& other, Size size);
        Code& cmpib(uint8_t value, const Param& other) { return this->cmpi(Data(value), other, Size::BYTE); }
        Code& cmpiw(uint16_t value, const Param& other) { return this->cmpi(Data(value), other, Size::WORD); }
        Code& cmpil(uint32_t value, const Param& other) { return this->cmpi(Data(value), other, Size::LONG); }
        
        Code& cmpa(const Param& value, const AddressRegister& reg);

        Code& tst(const Param& target, Size size);
        Code& tstb(const Param& target) { return this->tst(target, Size::BYTE); }
        Code& tstw(const Param& target) { return this->tst(target, Size::WORD); }
        Code& tstl(const Param& target) { return this->tst(target, Size::LONG); }

        Code& bra(uint16_t instructionCount = 0);
        Code& beq(uint16_t instructionCount = 0);
        Code& bne(uint16_t instructionCount = 0);
        Code& blt(uint16_t instructionCount = 0);
        Code& bgt(uint16_t instructionCount = 0);
        Code& dbra(const DataRegister& Dx, const std::string& label);

        Code& clr(const Register& reg, Size size);
        Code& clrb(const Register& reg) { return this->clr(reg, Size::BYTE); }
        Code& clrw(const Register& reg) { return this->clr(reg, Size::WORD); }
        Code& clrl(const Register& reg) { return this->clr(reg, Size::LONG); }

        Code& move(const Param& from, const Param& to, Size size);
        Code& moveb(uint8_t from, const Param& to) { return this->move(Data(from), to, Size::BYTE); }
        Code& movew(uint16_t from, const Param& to) { return this->move(Data(from), to, Size::WORD); }
        Code& movel(uint32_t from, const Param& to) { return this->move(Data(from), to, Size::LONG); }
        Code& moveb(const Param& from, const Param& to) { return this->move(from, to, Size::BYTE); }
        Code& movew(const Param& from, const Param& to) { return this->move(from, to, Size::WORD); }
        Code& movel(const Param& from, const Param& to) { return this->move(from, to, Size::LONG); }

        Code& moveq(uint8_t value, const Register& Dx);

        Code& movemToStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs);
        Code& movemFromStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs);

        Code& bset(const DataRegister& Dx, const Param& target);
        Code& btst(uint8_t bitID, const Param& target);
        Code& btst(const DataRegister& Dx, const Param& target);

        Code& subi(const Data& value, const Param& target, Size size);
        Code& subib(uint8_t value, const Param& target) { return this->subi(Data(value), target, Size::BYTE); }
        Code& subiw(uint16_t value, const Param& target) { return this->subi(Data(value), target, Size::WORD); }
        Code& subil(uint32_t value, const Param& target) { return this->subi(Data(value), target, Size::LONG); }

        Code& adda(uint32_t value, const AddressRegister& Ax);

        Code& lea(uint32_t value, const Register& Ax);

        Code& rts();
        Code& nop(uint16_t amount = 1);

        void label(const std::string& label) { _labels[label] = static_cast<uint32_t>(_bytes.size()); }
        const std::vector<uint8_t>& getBytes() const { return _bytes; }

    private:
        void resolveBranches();

        std::vector<uint8_t> _bytes;
        std::map<uint32_t, uint16_t> _pendingBranches;  // key = address of branch instruction, value = remaining instructions
        std::map<std::string, uint32_t> _labels;
    };
}

template<typename T>
inline constexpr asm68k::Data b_(T value)
{ return asm68k::Data(static_cast<uint8_t>(value)); }

template<typename T>
inline constexpr asm68k::Data w_(T value)
{ return asm68k::Data(static_cast<uint16_t>(value)); }

template<typename T>
inline constexpr asm68k::Data l_(T value)
{ return asm68k::Data(static_cast<uint32_t>(value)); }

template<typename T=void>
inline constexpr asm68k::DirectAddress addr_(uint32_t address) 
{ return asm68k::DirectAddress(address); }

template<typename T=void>
inline constexpr asm68k::AddressInRegister addrin_(const asm68k::AddressRegister& addressRegister)
{ return asm68k::AddressInRegister(addressRegister); }

template<typename T>
inline constexpr asm68k::AddressInRegister addroffset_(const asm68k::AddressRegister& addressRegister, T offset) 
{ return asm68k::AddressInRegister(addressRegister, offset); }

template<typename T=void>
inline constexpr asm68k::AddressWithIndex addroffset_(  const asm68k::AddressRegister& addressRegister,
                                                        const asm68k::DataRegister& offsetDataRegister,
                                                        asm68k::Size dataRegisterSize = asm68k::Size::LONG,
                                                        uint8_t additionnalOffset = 0) 
{ 
    return asm68k::AddressWithIndex(addressRegister, offsetDataRegister, dataRegisterSize, additionnalOffset);
}


#define reg_A0 asm68k::AddressRegister(0)
#define reg_A1 asm68k::AddressRegister(1)
#define reg_A2 asm68k::AddressRegister(2)
#define reg_A3 asm68k::AddressRegister(3)
#define reg_A4 asm68k::AddressRegister(4)
#define reg_A5 asm68k::AddressRegister(5)
#define reg_A6 asm68k::AddressRegister(6)
#define reg_A7 asm68k::AddressRegister(7)

#define reg_D0 asm68k::DataRegister(0)
#define reg_D1 asm68k::DataRegister(1)
#define reg_D2 asm68k::DataRegister(2)
#define reg_D3 asm68k::DataRegister(3)
#define reg_D4 asm68k::DataRegister(4)
#define reg_D5 asm68k::DataRegister(5)
#define reg_D6 asm68k::DataRegister(6)
#define reg_D7 asm68k::DataRegister(7)

#define reg_D0_D7 reg_D0,reg_D1,reg_D2,reg_D3,reg_D4,reg_D5,reg_D6,reg_D7
#define reg_A0_A6 reg_A0,reg_A1,reg_A2,reg_A3,reg_A4,reg_A5,reg_A6