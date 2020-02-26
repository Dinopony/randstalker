#include "MdCode.h"

#include <set>

namespace md
{
    void Code::addByte(uint8_t byte)
    {
        _bytes.push_back(byte);
    }

    void Code::addWord(uint16_t word)
    {
        this->addByte(static_cast<uint8_t>(word >> 8));
        this->addByte(static_cast<uint8_t>(word & 0xFF));
    }

    void Code::addLong(uint32_t longword)
    {
        this->addWord(static_cast<uint16_t>(longword >> 16));
        this->addWord(static_cast<uint16_t>(longword & 0xFFFF));
    }

    void Code::addBytes(const std::vector<uint8_t>& bytes)
    {
        for (uint8_t byte : bytes)
            this->addByte(byte);
    }

    void Code::addOpcode(uint16_t opcode)
    {
        this->resolveBranches();
        this->addWord(opcode);
    }

    Code& Code::jsr(uint32_t address)
    {
        this->addOpcode(0x4EB9);
        this->addLong(address);
        return *this;
    }

    Code& Code::jmp(uint32_t address)
    {
        this->addOpcode(0x4EF9);
        this->addLong(address);
        return *this;
    }

    Code& Code::cmpi(const ImmediateValue& value, const Param& other, Size size)
    {
        uint16_t sizeCode = 0x0;
        if (size == Size::WORD)
            sizeCode = 0x1;
        else if (size == Size::LONG)
            sizeCode = 0x2;

        uint16_t opcode = 0x0C00 + (sizeCode << 6) + other.getMXn();
        this->addOpcode(opcode);
        this->addBytes(value.getAdditionnalData());
        this->addBytes(other.getAdditionnalData());

        return *this;
    }

    Code& Code::cmpa(const Param& value, const AddressRegister& reg)
    {
        uint16_t opcode = 0xB1C0 + (reg.getXn() << 9) + value.getMXn();
        this->addOpcode(opcode);
        this->addBytes(value.getAdditionnalData());
        return *this;
    }

    Code& Code::tst(const Param& target, Size size)
    {
        uint16_t sizeCode = 0x0;
        if (size == Size::WORD)
            sizeCode = 0x1;
        else if (size == Size::LONG)
            sizeCode = 0x2;

        uint16_t opcode = 0x4A00 + (sizeCode << 6) + target.getMXn();
        this->addOpcode(opcode);
        this->addBytes(target.getAdditionnalData());

        return *this;
    }

    Code& Code::bra(uint16_t instructionCount)
    {
        this->addOpcode(0x6000);
        if (instructionCount > 0)
            _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

        return *this;
    }

    Code& Code::beq(uint16_t instructionCount)
    {
        this->addOpcode(0x6700);
        if (instructionCount > 0)
            _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

        return *this;
    }

    Code& Code::bne(uint16_t instructionCount)
    {
        this->addOpcode(0x6600);
        if (instructionCount > 0)
            _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

        return *this;
    }

    Code& Code::blt(uint16_t instructionCount)
    {
        this->addOpcode(0x6D00);
        if (instructionCount > 0)
            _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;
        return *this;
    }

    Code& Code::bgt(uint16_t instructionCount)
    {
        this->addOpcode(0x6E00);
        if (instructionCount > 0)
            _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;
        return *this;
    }

    Code& Code::dbra(const DataRegister& Dx, const std::string& label)
    {
        this->addOpcode(0x51C8 + Dx.getXn());

        uint32_t thisAddress = static_cast<uint32_t>(_bytes.size());
        uint32_t labelAddress = _labels.at(label);
        uint16_t offset = static_cast<uint16_t>(labelAddress - thisAddress);
        this->addWord(offset);

        return *this;
    }

    Code& Code::clr(const Register& Dx, Size size)
    {
        uint16_t sizeCode = 0x0;
        if (size == Size::WORD)
            sizeCode = 0x1;
        else if (size == Size::LONG)
            sizeCode = 0x2;

        uint16_t opcode = 0x4200 + (sizeCode << 6) + Dx.getMXn();
        this->addOpcode(opcode);

        return *this;
    }

    Code& Code::move(const Param& from, const Param& to, Size size)
    {
        uint16_t moveSizeCode = 0x1;
        if (size == Size::WORD)
            moveSizeCode = 0x3;
        else if (size == Size::LONG)
            moveSizeCode = 0x2;

        uint16_t opcode = (moveSizeCode << 12) + (to.getXnM() << 6) + from.getMXn();
        this->addOpcode(opcode);

        this->addBytes(from.getAdditionnalData());
        this->addBytes(to.getAdditionnalData());

        return *this;
    }

    Code& Code::moveq(uint8_t value, const Register& Dx)
    {
        uint16_t opcode = 0x7000 + (Dx.getXn() << 9) + value;
        this->addOpcode(opcode);
        return *this;
    }

    Code& Code::movemToStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs)
    {
        uint16_t registersToStore = 0x0000;
        for (const DataRegister& r : dataRegs)
            registersToStore |= (0x8000 >> r.getXn());
        for (const AddressRegister& r : addrRegs)
            registersToStore |= (0x0080 >> r.getXn());

        this->addOpcode(0x48E7);
        this->addWord(registersToStore); // 0xFFFE = D0-D7 and A0-A6
        return *this;
    }

    Code& Code::movemFromStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs)
    {
        uint16_t registersToStore = 0x0000;
        for (const DataRegister& r : dataRegs)
            registersToStore |= (0x0001 << r.getXn());
        for (const AddressRegister& r : addrRegs)
            registersToStore |= (0x0100 << r.getXn());

        this->addOpcode(0x4CDF);
        this->addWord(registersToStore); // 0x7FFF = A6-A0 and D7-D0
        return *this;
    }

    Code& Code::bset(uint8_t bitID, const Param& target)
    {
        this->addOpcode(0x08C0 + target.getMXn());
        this->addWord(static_cast<uint16_t>(bitID));
        this->addBytes(target.getAdditionnalData());
        return *this;
    }

    Code& Code::bset(const DataRegister& Dx, const Param& target)
    {
        uint16_t opcode = 0x01C0 + (Dx.getXn() << 9) + target.getMXn();
        this->addOpcode(opcode);
        this->addBytes(target.getAdditionnalData());
        return *this;
    }

    Code& Code::btst(uint8_t bitID, const Param& target)
    {
        this->addOpcode(0x0800 + target.getMXn());
        this->addWord(static_cast<uint16_t>(bitID));
        this->addBytes(target.getAdditionnalData());
        return *this;
    }

    Code& Code::btst(const DataRegister& Dx, const Param& target)
    {
        uint16_t opcode = 0x0100 + (Dx.getXn() << 9) + target.getMXn();
        this->addOpcode(opcode);
        this->addBytes(target.getAdditionnalData());
        return *this;
    }

    Code& Code::subi(const ImmediateValue& value, const Param& target, Size size)
    {
        uint16_t sizeCode = 0x0;
        if (size == Size::WORD)
            sizeCode = 0x1;
        else if (size == Size::LONG)
            sizeCode = 0x2;

        uint16_t opcode = 0x0400 + (sizeCode << 6) + target.getMXn();
        this->addOpcode(opcode);
        this->addBytes(value.getAdditionnalData());

        return *this;
    }

    Code& Code::adda(uint32_t value, const AddressRegister& Ax)
    {
        uint16_t opcode = 0xD1FC + (Ax.getXn() << 9);
        this->addOpcode(opcode);
        this->addLong(value);
        return *this;
    }

    Code& Code::lea(uint32_t value, const Register& Ax)
    {
        uint16_t opcode = 0x41F9 + (Ax.getXn() << 9);
        this->addOpcode(opcode);
        this->addLong(value);
        return *this;
    }

    Code& Code::rts()
    {
        this->resolveBranches();
        this->addWord(0x4E75);
        return *this;
    }

    Code& Code::nop(uint16_t amount)
    {
        this->resolveBranches();
        this->addWord(0x4E71);
        if (--amount > 0)
            this->nop(amount);
        return *this;
    }

    void Code::resolveBranches()
    {
        std::set<uint32_t> branchesToClean;

        for (auto& [branchAddress, remainingInstructions] : _pendingBranches)
        {
            if (--remainingInstructions == 0)
            {
                uint16_t addressOffset = static_cast<uint16_t>(_bytes.size()) - static_cast<uint16_t>(branchAddress);
                if (addressOffset > 0xFF)
                {
                    // Branch offset is more than one byte long, we need to add an optional displacement
                    _bytes.insert(_bytes.begin() + branchAddress, static_cast<uint8_t>(addressOffset >> 8));
                    _bytes.insert(_bytes.begin() + branchAddress + 1, static_cast<uint8_t>(addressOffset & 0xFF));
                }
                else
                {
                    // Branch offset is less than one byte long, we only need to increment opcode by the offset value
                    _bytes[static_cast<size_t>(branchAddress) - 1] += addressOffset;
                }

                branchesToClean.insert(branchAddress);
            }
        }

        for (uint32_t addr : branchesToClean)
            _pendingBranches.erase(addr);
    }

}