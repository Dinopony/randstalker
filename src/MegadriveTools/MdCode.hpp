#pragma once

#include <vector>
#include <map>
#include <string>
#include "MdTypes.hpp"

namespace md
{
    class Code {
    public:
        Code() {}

        void addByte(uint8_t byte);
        void addWord(uint16_t word);
        void addLong(uint32_t longword);
        void addBytes(const std::vector<uint8_t>& bytes);
        void addOpcode(uint16_t opcode);

        Code& bsr(uint8_t offset);
        Code& bsr(uint16_t offset);
        Code& jsr(uint32_t address);
        Code& jmp(uint32_t address);

        Code& cmp(const Param& value, const DataRegister& Dx, Size size);
        Code& cmpb(const Param& value, const DataRegister& Dx) { return this->cmp(value, Dx, Size::BYTE); }
        Code& cmpw(const Param& value, const DataRegister& Dx) { return this->cmp(value, Dx, Size::WORD); }
        Code& cmpl(const Param& value, const DataRegister& Dx) { return this->cmp(value, Dx, Size::LONG); }

        Code& cmpi(const ImmediateValue& value, const Param& other, Size size);
        Code& cmpib(uint8_t value, const Param& other) { return this->cmpi(ImmediateValue(value), other, Size::BYTE); }
        Code& cmpiw(uint16_t value, const Param& other) { return this->cmpi(ImmediateValue(value), other, Size::WORD); }
        Code& cmpil(uint32_t value, const Param& other) { return this->cmpi(ImmediateValue(value), other, Size::LONG); }
        
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
        Code& bmi(uint16_t instructionCount = 0);
        Code& bpl(uint16_t instructionCount = 0);
        Code& bcc(uint16_t instructionCount = 0);
        Code& bra(const std::string& label);
        Code& dbra(const DataRegister& Dx, const std::string& label);

        Code& clr(const Param& param, Size size);
        Code& clrb(const Param& param) { return this->clr(param, Size::BYTE); }
        Code& clrw(const Param& param) { return this->clr(param, Size::WORD); }
        Code& clrl(const Param& param) { return this->clr(param, Size::LONG); }

        Code& move(const Param& from, const Param& to, Size size);
        Code& moveb(uint8_t from, const Param& to) { return this->move(ImmediateValue(from), to, Size::BYTE); }
        Code& movew(uint16_t from, const Param& to) { return this->move(ImmediateValue(from), to, Size::WORD); }
        Code& movel(uint32_t from, const Param& to) { return this->move(ImmediateValue(from), to, Size::LONG); }
        Code& moveb(const Param& from, const Param& to) { return this->move(from, to, Size::BYTE); }
        Code& movew(const Param& from, const Param& to) { return this->move(from, to, Size::WORD); }
        Code& movel(const Param& from, const Param& to) { return this->move(from, to, Size::LONG); }

        Code& moveq(uint8_t value, const DataRegister& Dx);

        Code& addq(uint8_t value, const Register& Rx, Size size);
        Code& addqb(uint8_t value, const Register& Rx) { return this->addq(value, Rx, Size::BYTE); }
        Code& addqw(uint8_t value, const Register& Rx) { return this->addq(value, Rx, Size::WORD); }
        Code& addql(uint8_t value, const Register& Rx) { return this->addq(value, Rx, Size::LONG); }

        Code& movemToStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs);
        Code& movemFromStack(const std::vector<DataRegister>& dataRegs, const std::vector<AddressRegister>& addrRegs);

        Code& bset(uint8_t bitID, const Param& target);
        Code& bset(const DataRegister& Dx, const Param& target);
        Code& btst(uint8_t bitID, const Param& target);
        Code& btst(const DataRegister& Dx, const Param& target);

        Code& subi(const ImmediateValue& value, const Param& target, Size size);
        Code& subib(uint8_t value, const Param& target) { return this->subi(ImmediateValue(value), target, Size::BYTE); }
        Code& subiw(uint16_t value, const Param& target) { return this->subi(ImmediateValue(value), target, Size::WORD); }
        Code& subil(uint32_t value, const Param& target) { return this->subi(ImmediateValue(value), target, Size::LONG); }

        Code& mulu(const Param& value, const DataRegister& Dx);
        Code& divu(const Param& value, const DataRegister& Dx);

        Code& adda(const Param& value, const AddressRegister& Ax);
        Code& adda(uint32_t value, const AddressRegister& Ax) { return this->adda(ImmediateValue(value), Ax); }

        Code& lea(uint32_t value, const Register& Ax);

        Code& andToDx(const Param& from, const DataRegister& to, Size size);
        Code& andb(const Param& from, const DataRegister& to) { return this->andToDx(from, to, Size::BYTE); }
        Code& andw(const Param& from, const DataRegister& to) { return this->andToDx(from, to, Size::WORD); }
        Code& andl(const Param& from, const DataRegister& to) { return this->andToDx(from, to, Size::LONG); }

        Code& andi(const ImmediateValue& value, const Param& target, Size size);
        Code& andib(uint8_t value, const Param& target) { return this->andi(ImmediateValue(value), target, Size::BYTE); }
        Code& andiw(uint16_t value, const Param& target) { return this->andi(ImmediateValue(value), target, Size::WORD); }
        Code& andil(uint32_t value, const Param& target) { return this->andi(ImmediateValue(value), target, Size::LONG); }
        Code& oriToCCR(uint8_t value);

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
