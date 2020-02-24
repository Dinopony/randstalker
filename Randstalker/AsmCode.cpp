#include "AsmCode.h"

#include <set>

static uint16_t bytesToWord(uint8_t msb, uint8_t lsb)
{
    return (static_cast<uint16_t>(msb) << 8) + static_cast<uint16_t>(lsb);
}

void AsmCode::addByte(uint8_t byte)
{
    _bytes.push_back(byte);
}

void AsmCode::addWord(uint16_t word)
{
    this->addByte(static_cast<uint8_t>(word >> 8));
    this->addByte(static_cast<uint8_t>(word & 0xFF));
}

void AsmCode::addLong(uint32_t longword)
{
    this->addWord(static_cast<uint16_t>(longword >> 16));
    this->addWord(static_cast<uint16_t>(longword & 0xFFFF));
}

void AsmCode::addOpcode(uint16_t opcode)
{
    this->resolveBranches();
    this->addWord(opcode);
}

AsmCode& AsmCode::jsr(uint32_t address)
{
    this->addOpcode(0x4EB9);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::jmp(uint32_t address)
{
    this->addOpcode(0x4EF9);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::cmpib(uint8_t comparedValue, uint32_t address)
{
    this->addOpcode(0x0C39);
    this->addByte(0x00);
    this->addByte(comparedValue);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::cmpibWithDx(uint8_t comparedValue, uint8_t Dx)
{
    this->addOpcode(0x0C00 + Dx);
    this->addByte(0x00);
    this->addByte(comparedValue);
    return *this;
}

AsmCode& AsmCode::cmpiwWithDx(uint16_t comparedValue, uint8_t Dx)
{
    this->addOpcode(0x0C40 + Dx);
    this->addWord(comparedValue);
    return *this;
}

AsmCode& AsmCode::cmpaWithAx(uint32_t comparedAddr, uint8_t Ax)
{
    this->addOpcode(bytesToWord(0xB1 + (Ax << 1), 0xFC));
    this->addLong(comparedAddr);
    return *this;
}

AsmCode& AsmCode::btst(uint8_t bitID, uint32_t address)
{
    this->addOpcode(0x0839);
    this->addByte(0x00);
    this->addByte(bitID);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::bra(uint16_t instructionCount)
{
    this->addOpcode(0x6000);
    if (instructionCount > 0)
        _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

    return *this;
}

AsmCode& AsmCode::beq(uint16_t instructionCount)
{
    this->addOpcode(0x6700);
    if(instructionCount > 0)
        _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

    return *this;
}

AsmCode& AsmCode::bne(uint16_t instructionCount)
{
    this->addOpcode(0x6600);
    if (instructionCount > 0)
        _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;

    return *this;
}

AsmCode& AsmCode::blt(uint16_t instructionCount)
{
    this->addOpcode(0x6D00);
    if (instructionCount > 0)
        _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;
    return *this;
}

AsmCode& AsmCode::bgt(uint16_t instructionCount)
{
    this->addOpcode(0x6E00);
    if (instructionCount > 0)
        _pendingBranches[static_cast<uint32_t>(_bytes.size())] = instructionCount;
    return *this;
}

AsmCode& AsmCode::clrwDx(uint8_t Dx)
{
    this->addOpcode(0x4240 + Dx);
    return *this;
}

AsmCode& AsmCode::moveb(uint8_t value, uint32_t address)
{
    this->addOpcode(0x13FC);
    this->addByte(0x00);
    this->addByte(value);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::movew(uint16_t value, uint32_t address)
{
    this->addOpcode(0x33FC);
    this->addWord(value);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::movel(uint32_t value, uint32_t address)
{
    this->addOpcode(0x23FC);
    this->addLong(value);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::movewToDx(uint16_t value, uint8_t Dx)
{
    this->addOpcode(bytesToWord(0x30 + (Dx << 1), 0x3C));
    this->addWord(value);
    return *this;
}

AsmCode& AsmCode::movewToAddressInAx(uint16_t value, uint8_t Ax)
{
    this->addOpcode(bytesToWord(0x30 + (Ax << 1), 0xBC));
    this->addWord(value);
    return *this;
}

AsmCode& AsmCode::movewRegToAddress(uint8_t Dx, uint32_t address)
{
    this->addOpcode(0x33C0 + Dx);
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::moveqToDx(uint8_t value, uint8_t Dx)
{
    this->addOpcode(bytesToWord(0x70 + (Dx << 1), value));
    return *this;
}

AsmCode& AsmCode::movemToStack()
{
    this->addOpcode(0x48E7);
    this->addWord(0xFFFE); // D0-D7 and A0-A6
    return *this;
}

AsmCode& AsmCode::movemFromStack()
{
    this->addOpcode(0x4CDF);
    this->addWord(0x7FFF); // A6-A0 and D7-D0
    return *this;
}

AsmCode& AsmCode::bsetDxOnAddress(uint8_t Dx, uint32_t address)
{
    this->addOpcode(bytesToWord(0x01 + (Dx << 1), 0xF9));
    this->addLong(address);
    return *this;
}

AsmCode& AsmCode::subibFromDx(uint8_t value, uint8_t Dx)
{
    this->addOpcode(0x0400 + Dx);
    this->addWord(bytesToWord(0x00, value));
    return *this;
}

AsmCode& AsmCode::addaToAx(uint32_t value, uint8_t Ax)
{
    this->addOpcode(bytesToWord(0xD1 + (Ax << 1), 0xFC));
    this->addLong(value);
    return *this;
}

AsmCode& AsmCode::leaToAx(uint32_t value, uint8_t Ax)
{
    this->addOpcode(bytesToWord(0x41 + (Ax << 1), 0xF9));
    this->addLong(value);
    return *this;
}

AsmCode& AsmCode::rts()
{
    this->resolveBranches();
    this->addWord(0x4E75);
    return *this;
}

AsmCode& AsmCode::nop(uint16_t amount)
{
    this->resolveBranches();
    this->addWord(0x4E71);
    if (--amount > 0)
        this->nop(amount);
    return *this;
}

void AsmCode::resolveBranches()
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
                _bytes[static_cast<size_t>(branchAddress)-1] += addressOffset;
            }

            branchesToClean.insert(branchAddress);
        }
    }

    for (uint32_t addr : branchesToClean)
        _pendingBranches.erase(addr);
}
