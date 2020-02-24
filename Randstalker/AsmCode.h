#pragma once

#include <vector>
#include <map>

constexpr uint8_t REG_D0 = 0x0;
constexpr uint8_t REG_D1 = 0x1;
constexpr uint8_t REG_D2 = 0x2;
constexpr uint8_t REG_D3 = 0x3;
constexpr uint8_t REG_D4 = 0x4;
constexpr uint8_t REG_D5 = 0x5;
constexpr uint8_t REG_D6 = 0x6;
constexpr uint8_t REG_D7 = 0x7;

constexpr uint8_t REG_A0 = 0x0;
constexpr uint8_t REG_A1 = 0x1;
constexpr uint8_t REG_A2 = 0x2;
constexpr uint8_t REG_A3 = 0x3;
constexpr uint8_t REG_A4 = 0x4;
constexpr uint8_t REG_A5 = 0x5;
constexpr uint8_t REG_A6 = 0x6;
constexpr uint8_t REG_A7 = 0x7;

class AsmCode {
public:
	AsmCode() {}

    void addByte(uint8_t byte);
    void addWord(uint16_t word);
    void addLong(uint32_t longword);
    void addOpcode(uint16_t opcode);

    AsmCode& jsr(uint32_t address);
    AsmCode& jmp(uint32_t address);

    AsmCode& cmpib(uint8_t comparedValue, uint32_t address);
    AsmCode& cmpibWithDx(uint8_t comparedValue, uint8_t Dx);
    AsmCode& cmpiwWithDx(uint16_t comparedValue, uint8_t Dx);
    AsmCode& cmpaWithAx(uint32_t comparedAddr, uint8_t Ax);
    AsmCode& btst(uint8_t bitID, uint32_t address);

    AsmCode& bra(uint16_t instructionCount = 0);
    AsmCode& beq(uint16_t instructionCount = 0);
    AsmCode& bne(uint16_t instructionCount = 0);
    AsmCode& blt(uint16_t instructionCount = 0);
    AsmCode& bgt(uint16_t instructionCount = 0);

    AsmCode& clrwDx(uint8_t Dx);

    AsmCode& moveb(uint8_t value, uint32_t address);
    AsmCode& movew(uint16_t value, uint32_t address);
    AsmCode& movel(uint32_t value, uint32_t address);
    AsmCode& movewToDx(uint16_t value, uint8_t Dx);
    AsmCode& movewToAddressInAx(uint16_t value, uint8_t Ax);
    AsmCode& movewRegToAddress(uint8_t Dx, uint32_t address);
    AsmCode& moveqToDx(uint8_t value, uint8_t Dx);

    AsmCode& movemToStack();
    AsmCode& movemFromStack();

    AsmCode& bsetDxOnAddress(uint8_t Dx, uint32_t address);

    AsmCode& subibFromDx(uint8_t value, uint8_t Dx);
    AsmCode& addaToAx(uint32_t value, uint8_t Ax);

    AsmCode& leaToAx(uint32_t value, uint8_t Ax);

    AsmCode& rts();
    AsmCode& nop(uint16_t amount = 1);

    const std::vector<uint8_t>& getBytes() const { return _bytes; }

private:
    void resolveBranches();

	std::vector<uint8_t> _bytes;
    std::map<uint32_t, uint16_t> _pendingBranches;  // key = address of branch instruction, value = remaining instructions
};