//
// Created by Đức Bùi on 14/5/2026.
//

#include "cpu.hpp"


/**
 * AND - Logical AND
 * A,Z,N = A&M
 * A logical AND is performed, bit by bit, on the accumulator contents using the contents of a byte of memory.
 * Processor Status after use:
 * Z | Set if A == 0
 */
void CPU::AND(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto data = bus.read(addr);
    register_a_ &= data;
    update_negative(data & register_a_);
    update_zero(data & register_a_);
}
void CPU::ASL(AddressingMode mode)
{
    if (mode == Accumulator)
    {
        if (register_a_ >> 7 == 1)
            status_.set(Flag::CARRY);
        else
            status_.remove(Flag::CARRY);
        register_a_ <<= 1;
        update_zero(register_a_);
        update_negative(register_a_);
        return;
    }
    auto addr = get_operand_address(mode);
    auto data = bus.read(addr);
    if (data >> 7 == 1)
        status_.set(Flag::CARRY);
    else
        status_.remove(Flag::CARRY);
    data <<= 1;
    bus.write(addr, data);

    (void)update_zero(data);
    (void)update_negative(data);
}
void CPU::BCC(AddressingMode mode)
{
    branch_if(!status_.is_set(Flag::CARRY));
}

void CPU::BCS(AddressingMode mode)
{
    branch_if(status_.is_set(Flag::CARRY));
}
void CPU::BEQ(AddressingMode mode)
{
    branch_if(status_.is_set(Flag::ZERO));
}
void CPU::BIT(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    update_zero(value & register_a_);

    if ((value & 0b01000000) > 0)
        status_.set(Flag::OVERFLOWED);
    else
        status_.remove(Flag::OVERFLOWED);

    if ((value & 0b10000000) > 0)
        status_.set(Flag::NEGATIVE);
    else
        status_.remove(Flag::NEGATIVE);
}
void CPU::BMI(AddressingMode mode)
{
    branch_if(status_.is_set(Flag::NEGATIVE));
}
void CPU::BNE(AddressingMode mode)
{
    branch_if(!status_.is_set(Flag::ZERO));
}
void CPU::BPL(AddressingMode mode)
{
    branch_if(!status_.is_set(Flag::NEGATIVE));
}
void CPU::BRK(AddressingMode mode)
{
    status_.set(Flag::BREAK);
}
void CPU::BVC(AddressingMode mode)
{
    branch_if(!status_.is_set(Flag::OVERFLOWED));
}
void CPU::BVS(AddressingMode mode)
{
    branch_if(status_.is_set(Flag::OVERFLOWED));
}
void CPU::CLC(AddressingMode mode)
{
    status_.remove(Flag::CARRY);
}
void CPU::CLD(AddressingMode mode)
{
    status_.remove(Flag::DECIMAL);
}
void CPU::CLI(AddressingMode mode)
{
    status_.remove(Flag::INTERRUPT);
}
void CPU::CLV(AddressingMode mode)
{
    status_.remove(Flag::OVERFLOWED);
}
void CPU::CMP(AddressingMode mode)
{
    auto value = bus.read(get_operand_address(mode));
    const auto result = static_cast<uint8_t>(register_a_ - value);
    update_zero(result);
    update_negative(result);
    (register_a_ >= value) ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
}
void CPU::CPX(AddressingMode mode)
{
    auto value = bus.read(get_operand_address(mode));
    const auto result = static_cast<uint8_t>(register_x_ - value);
    update_zero(result);
    update_negative(result);
    (register_x_ >= value) ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
}
void CPU::CPY(AddressingMode mode)
{
    auto value = bus.read(get_operand_address(mode));
    const auto result = static_cast<uint8_t>(register_y_ - value);
    update_zero(result);
    update_negative(result);
    (register_y_ >= value) ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
}
void CPU::DEC(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    update_zero(value - 1);
    update_negative(value - 1);
    bus.write(addr, value - 1);
}
void CPU::DEX(AddressingMode mode)
{
    update_negative(register_x_ - 1);
    update_zero(register_x_ - 1);
    register_x_ -= 1;
}
void CPU::DEY(AddressingMode mode)
{
    update_negative(register_y_ - 1);
    update_zero(register_y_ - 1);
    register_y_ -= 1;
}
void CPU::EOR(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    update_negative(register_a_ ^ value);
    update_zero(register_a_ ^ value);
    register_a_ ^= value;
}
void CPU::INC(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    update_negative(value + 1);
    update_zero(value + 1);
    bus.write(addr, value + 1);
}
void CPU::INX(AddressingMode mode)
{
    register_x_ += 1;
    update_zero(register_x_);
    update_negative(register_x_);
}
void CPU::INY(AddressingMode mode)
{
    register_y_ += 1;
    update_zero(register_y_);
    update_negative(register_y_);
}
void CPU::JMP(AddressingMode mode)
{
    if (mode == AddressingMode::Absolute)
    {
        program_counter_ = bus.read_u16(program_counter_);
    }
    else if (mode == AddressingMode::Indirect)
    {
        auto mem_addr = bus.read_u16(program_counter_);
        program_counter_ = [this, mem_addr]()
        {
            if ((mem_addr & 0x00ffu) == 0x00ffu)
            {
                uint16_t lo = this->bus.read(mem_addr);
                uint16_t hi = this->bus.read(mem_addr & 0xFF00);
                return (static_cast<uint16_t>(static_cast<uint16_t>(hi << 8) | lo));
            }
            return this->bus.read_u16(mem_addr);
        }();
    };
}
void CPU::JSR(AddressingMode mode)
{
    stack_push_u16(program_counter_ + 2 - 1);
    const auto target_address = bus.read_u16(program_counter_);
    program_counter_ = target_address;
}
void CPU::LDA(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    register_a_ = value;
    update_negative(register_a_);
    update_zero(register_a_);
}
void CPU::LDX(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    register_x_ = value;
    update_negative(register_x_);
    update_zero(register_x_);
}
void CPU::LDY(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    register_y_ = value;
    update_negative(register_y_);
    update_zero(register_y_);
}

void CPU::LSR(AddressingMode mode)
{
    if (mode == AddressingMode::Accumulator)
    {
        if ((register_a_ & 1) == 1) status_.set(CARRY);
        else status_.remove(CARRY);
        register_a_ = register_a_ >> 1;
        update_zero(register_a_);
        update_negative(register_a_);
        return;
    }
    auto addr = get_operand_address(mode);
    auto value = bus.read(addr);
    if ((value & 1) == 1) status_.set(CARRY);
    else status_.remove(CARRY);
    value = value >> 1;
    update_zero(value);
    update_negative(value);
    bus.write(addr, value);
}
void CPU::NOP(AddressingMode mode)
{
    return;
}
void CPU::ORA(AddressingMode mode)
{
    auto addr = get_operand_address(mode);
    auto data = bus.read(addr);
    register_a_ = (data | register_a_);
    update_negative(register_a_);
    update_zero(register_a_);
}
void CPU::PHA(AddressingMode mode)
{
    stack_push(register_a_);
}
void CPU::PHP(AddressingMode mode)
{
    auto flags = BitFlags(status_.status);
    flags.set(BREAK);
    flags.set(UNUSED);
    stack_push(flags.status);
}
void CPU::PLA(AddressingMode mode)
{
    auto data = stack_pop();
    set_register_a(data);
}
void CPU::PLP(AddressingMode mode)
{
    status_.status = stack_pop();
    status_.remove(BREAK);
    status_.set(UNUSED);
}
void CPU::ROL(AddressingMode mode)
{
    if (mode == AddressingMode::Accumulator)
    {
        auto old_carry = status_.is_set(Flag::CARRY);
        if (register_a_ >> 7 == 1)
            status_.set(Flag::CARRY);
        else
            status_.remove(Flag::CARRY);

        register_a_ <<= 1;
        if (old_carry)
            register_a_ |= 1;
        update_zero(register_a_);
        update_negative(register_a_);
        return;
    }
    auto addr = get_operand_address(mode);
    auto data = bus.read(addr);
    auto old_carry = status_.is_set(Flag::CARRY);

    if (data >> 7 == 1)
        status_.set(Flag::CARRY);
    else
        status_.remove(Flag::CARRY);
    data <<= 1;
    if (old_carry)
        data |= 1;
    update_negative(data);
    update_zero(data);
    bus.write(addr, data);
}
void CPU::ROR(AddressingMode mode)
{
    if (mode == AddressingMode::Accumulator)
    {
        auto old_carry = status_.is_set(Flag::CARRY);
        if ((register_a_ & 1) == 1)
            status_.set(Flag::CARRY);
        else
            status_.remove(Flag::CARRY);

        register_a_ >>= 1;
        if (old_carry)
            register_a_ |= 0b10000000;
        update_zero(register_a_);
        update_negative(register_a_);
        return;
    }
    auto addr = get_operand_address(mode);
    auto data = bus.read(addr);
    auto old_carry = status_.is_set(Flag::CARRY);

    if ((data & 1) == 1)
        status_.set(Flag::CARRY);
    else
        status_.remove(Flag::CARRY);
    data >>= 1;
    if (old_carry)
        data |= 0b10000000;
    update_negative(data);
    update_zero(data);
    bus.write(addr, data);
}
void CPU::RTI(AddressingMode mode)
{
    status_.status = stack_pop();
    status_.remove(Flag::BREAK);
    status_.set(Flag::UNUSED);

    program_counter_ = stack_pop_u16();
}
void CPU::RTS(AddressingMode mode)
{
    program_counter_ = stack_pop_u16() + 1;
}
void CPU::SBC(AddressingMode mode)
{
    auto address = get_operand_address(mode);
    auto value = bus.read(address);

    add_to_register_a(-value-1);
}
void CPU::SEC(AddressingMode mode)
{
    status_.set(Flag::CARRY);
}
void CPU::SED(AddressingMode mode)
{
    status_.set(Flag::DECIMAL);
}
void CPU::SEI(AddressingMode mode)
{
    status_.set(Flag::INTERRUPT);
}
void CPU::STA(AddressingMode mode)
{
    auto address = get_operand_address(mode);

    bus.write(address, register_a_);
}
void CPU::STX(AddressingMode mode)
{
    auto address = get_operand_address(mode);

    bus.write(address, register_x_);
}
void CPU::STY(AddressingMode mode)
{
    auto address = get_operand_address(mode);

    bus.write(address, register_y_);
}
void CPU::TAX(AddressingMode mode)
{
    register_x_ = register_a_;
    update_negative(register_x_);
    update_zero(register_x_);
}
void CPU::TAY(AddressingMode mode)
{
    register_y_ = register_a_;
    update_negative(register_y_);
    update_zero(register_y_);
}

void CPU::TSX(AddressingMode mode)
{
    register_x_ = stack_pointer_;
    update_negative(register_x_);
    update_zero(register_x_);
}
void CPU::TXA(AddressingMode mode)
{
    register_a_ = register_x_;
    update_negative(register_a_);
    update_zero(register_a_);
}
void CPU::TXS(AddressingMode mode)
{
    stack_pointer_ = register_x_;
}
void CPU::TYA(AddressingMode mode)
{
    register_a_ = register_y_;
    update_negative(register_a_);
    update_zero(register_a_);
}
