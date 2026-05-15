#include "cpu.hpp"

#include <cassert>
#include <functional>
#include <iomanip>
#include <sstream>
#include <utility>
#include "addressing_mode.hpp"
#include "flag.hpp"
#include "opcodes.hpp"

namespace {
std::string hex_u8(uint8_t value) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0') << std::setw(2)
        << static_cast<int>(value);
    return out.str();
}

std::string hex_u16(uint16_t value) {
    std::ostringstream out;
    out << std::uppercase << std::hex << std::setfill('0') << std::setw(4)
        << static_cast<int>(value);
    return out.str();
}

std::string instruction_bytes(const Bus& bus, uint16_t pc, uint8_t len) {
    std::ostringstream out;
    for (uint8_t i = 0; i < 3; ++i) {
        if (i < len) {
            out << hex_u8(bus.read(static_cast<uint16_t>(pc + i)));
        } else {
            out << "  ";
        }
        if (i < 2) {
            out << ' ';
        }
    }
    return out.str();
}

std::string trace_u8(const Bus& bus, uint16_t address) {
    if (address >= PPU_REGISTERS && address <= PPU_REGISTERS_MIRROR_END) {
        return "??";
    }
    return hex_u8(bus.read(address));
}
}

// CPU::CPU() = default;
CPU::~CPU() = default;

void CPU::interpret(std::vector<uint8_t> program)
{
    load(std::move(program));
    interpret_with_callback([](CPU *) {});
}

void CPU::load_and_run(std::vector<uint8_t> program)
{
    interpret(std::move(program));
}

void CPU::mem_write(uint16_t address, uint8_t data)
{
    bus.write(address, data);
}

uint8_t CPU::register_a() const
{
    return register_a_;
}

uint8_t CPU::register_x() const
{
    return register_x_;
}

uint8_t CPU::status() const
{
    return status_.status;
}

void CPU::set_register_a_for_test(uint8_t data)
{
    register_a_ = data;
}

void CPU::set_register_x_for_test(uint8_t data)
{
    register_x_ = data;
}

void CPU::set_logger(Logger* logger)
{
    logger_ = logger;
}

void CPU::reset() {
    register_a_ = 0;
    register_x_ = 0;
    register_y_ = 0;
    stack_pointer_ = STACK_RESET;
    cycles_ = 0;
    // program_counter_ = bus.read_u16(0xfffc);
    program_counter_ = 0xC000;
}

void CPU::interpret_with_callback(const std::function<void(CPU *)>& callback)
{

    while (true)
    {
        const uint8_t opcode = bus.read(program_counter_++);

        auto program_counter_state = program_counter_;


        if (const auto instruction = InstructionTable.find(opcode);
            instruction != InstructionTable.end())
        {

            trace_instruction(static_cast<uint16_t>(program_counter_ - 1), instruction->second);
            const auto mode = instruction->second.mode;
            const auto& mnemonic = instruction->second.mnemonic;
            bool handled = true;

            if (mnemonic == "*NOP")
                NOP(mode);
            else if (mnemonic == "*ALR")
                ALR(mode);
            else if (mnemonic == "*ANC")
                ANC(mode);
            else if (mnemonic == "*ARR")
                ARR(mode);
            else if (mnemonic == "*AXS")
                AXS(mode);
            else if (mnemonic == "*DCP")
                DCP(mode);
            else if (mnemonic == "*ISB")
                ISB(mode);
            else if (mnemonic == "*LAX")
                LAX(mode);
            else if (mnemonic == "*RLA")
                RLA(mode);
            else if (mnemonic == "*RRA")
                RRA(mode);
            else if (mnemonic == "*SAX")
                SAX(mode);
            else if (mnemonic == "*SBC")
                SBC(mode);
            else if (mnemonic == "*SHX")
                SHX(mode);
            else if (mnemonic == "*SHY")
                SHY(mode);
            else if (mnemonic == "*SLO")
                SLO(mode);
            else if (mnemonic == "*SRE")
                SRE(mode);
            else
                handled = false;

            if (!handled) switch (opcode)
            {
            case 0x69u:
                ADC(instruction->second.mode);
                break;
            case 0x65u:
                ADC(instruction->second.mode);
                break;
            case 0x75u:
                ADC(instruction->second.mode);
                break;
            case 0x6Du:
                ADC(instruction->second.mode);
                break;
            case 0x7Du:
                ADC(instruction->second.mode);
                break;
            case 0x79u:
                ADC(instruction->second.mode);
                break;
            case 0x61u:
                ADC(instruction->second.mode);
                break;
            case 0x71u:
                ADC(instruction->second.mode);
                break;

            case 0x29u:
                AND(instruction->second.mode);
                break;
            case 0x25u:
                AND(instruction->second.mode);
                break;
            case 0x35u:
                AND(instruction->second.mode);
                break;
            case 0x2Du:
                AND(instruction->second.mode);
                break;
            case 0x3Du:
                AND(instruction->second.mode);
                break;
            case 0x39u:
                AND(instruction->second.mode);
                break;
            case 0x21u:
                AND(instruction->second.mode);
                break;
            case 0x31u:
                AND(instruction->second.mode);
                break;

            case 0x0Au:
                ASL(instruction->second.mode);
                break;
            case 0x06u:
                ASL(instruction->second.mode);
                break;
            case 0x16u:
                ASL(instruction->second.mode);
                break;
            case 0x0Eu:
                ASL(instruction->second.mode);
                break;
            case 0x1Eu:
                ASL(instruction->second.mode);
                break;

            case 0x90u:
                BCC(instruction->second.mode);
                break;
            case 0xB0u:
                BCS(instruction->second.mode);
                break;
            case 0xF0u:
                BEQ(instruction->second.mode);
                break;
            case 0x24u:
                BIT(instruction->second.mode);
                break;
            case 0x2Cu:
                BIT(instruction->second.mode);
                break;
            case 0x30u:
                BMI(instruction->second.mode);
                break;
            case 0xD0u:
                BNE(instruction->second.mode);
                break;
            case 0x10u:
                BPL(instruction->second.mode);
                break;
            case 0x00u:
                BRK(instruction->second.mode);
                return;
            case 0x50u:
                BVC(instruction->second.mode);
                break;
            case 0x70u:
                BVS(instruction->second.mode);
                break;

            case 0x18u:
                CLC(instruction->second.mode);
                break;
            case 0xD8u:
                CLD(instruction->second.mode);
                break;
            case 0x58u:
                CLI(instruction->second.mode);
                break;
            case 0xB8u:
                CLV(instruction->second.mode);
                break;

            case 0xC9u:
                CMP(instruction->second.mode);
                break;
            case 0xC5u:
                CMP(instruction->second.mode);
                break;
            case 0xD5u:
                CMP(instruction->second.mode);
                break;
            case 0xCDu:
                CMP(instruction->second.mode);
                break;
            case 0xDDu:
                CMP(instruction->second.mode);
                break;
            case 0xD9u:
                CMP(instruction->second.mode);
                break;
            case 0xC1u:
                CMP(instruction->second.mode);
                break;
            case 0xD1u:
                CMP(instruction->second.mode);
                break;

            case 0xE0u:
                CPX(instruction->second.mode);
                break;
            case 0xE4u:
                CPX(instruction->second.mode);
                break;
            case 0xECu:
                CPX(instruction->second.mode);
                break;

            case 0xC0u:
                CPY(instruction->second.mode);
                break;
            case 0xC4u:
                CPY(instruction->second.mode);
                break;
            case 0xCCu:
                CPY(instruction->second.mode);
                break;

            case 0xC6u:
                DEC(instruction->second.mode);
                break;
            case 0xD6u:
                DEC(instruction->second.mode);
                break;
            case 0xCEu:
                DEC(instruction->second.mode);
                break;
            case 0xDEu:
                DEC(instruction->second.mode);
                break;

            case 0xCAu:
                DEX(instruction->second.mode);
                break;
            case 0x88u:
                DEY(instruction->second.mode);
                break;

            case 0x49u:
                EOR(instruction->second.mode);
                break;
            case 0x45u:
                EOR(instruction->second.mode);
                break;
            case 0x55u:
                EOR(instruction->second.mode);
                break;
            case 0x4Du:
                EOR(instruction->second.mode);
                break;
            case 0x5Du:
                EOR(instruction->second.mode);
                break;
            case 0x59u:
                EOR(instruction->second.mode);
                break;
            case 0x41u:
                EOR(instruction->second.mode);
                break;
            case 0x51u:
                EOR(instruction->second.mode);
                break;

            case 0xE6u:
                INC(instruction->second.mode);
                break;
            case 0xF6u:
                INC(instruction->second.mode);
                break;
            case 0xEEu:
                INC(instruction->second.mode);
                break;
            case 0xFEu:
                INC(instruction->second.mode);
                break;

            case 0xE8u:
                INX(instruction->second.mode);
                break;
            case 0xC8u:
                INY(instruction->second.mode);
                break;

            case 0x4Cu:
                JMP(instruction->second.mode);
                break;
            case 0x6Cu:
                JMP(instruction->second.mode);
                break;
            case 0x20u:
                JSR(instruction->second.mode);
                break;

            case 0xA9u:
                LDA(instruction->second.mode);
                break;
            case 0xA5u:
                LDA(instruction->second.mode);
                break;
            case 0xB5u:
                LDA(instruction->second.mode);
                break;
            case 0xADu:
                LDA(instruction->second.mode);
                break;
            case 0xBDu:
                LDA(instruction->second.mode);
                break;
            case 0xB9u:
                LDA(instruction->second.mode);
                break;
            case 0xA1u:
                LDA(instruction->second.mode);
                break;
            case 0xB1u:
                LDA(instruction->second.mode);
                break;

            case 0xA2u:
                LDX(instruction->second.mode);
                break;
            case 0xA6u:
                LDX(instruction->second.mode);
                break;
            case 0xB6u:
                LDX(instruction->second.mode);
                break;
            case 0xAEu:
                LDX(instruction->second.mode);
                break;
            case 0xBEu:
                LDX(instruction->second.mode);
                break;

            case 0xA0u:
                LDY(instruction->second.mode);
                break;
            case 0xA4u:
                LDY(instruction->second.mode);
                break;
            case 0xB4u:
                LDY(instruction->second.mode);
                break;
            case 0xACu:
                LDY(instruction->second.mode);
                break;
            case 0xBCu:
                LDY(instruction->second.mode);
                break;

            case 0x4Au:
                LSR(instruction->second.mode);
                break;
            case 0x46u:
                LSR(instruction->second.mode);
                break;
            case 0x56u:
                LSR(instruction->second.mode);
                break;
            case 0x4Eu:
                LSR(instruction->second.mode);
                break;
            case 0x5Eu:
                LSR(instruction->second.mode);
                break;

            case 0xEAu:
                NOP(instruction->second.mode);
                break;

            case 0x09u:
                ORA(instruction->second.mode);
                break;
            case 0x05u:
                ORA(instruction->second.mode);
                break;
            case 0x15u:
                ORA(instruction->second.mode);
                break;
            case 0x0Du:
                ORA(instruction->second.mode);
                break;
            case 0x1Du:
                ORA(instruction->second.mode);
                break;
            case 0x19u:
                ORA(instruction->second.mode);
                break;
            case 0x01u:
                ORA(instruction->second.mode);
                break;
            case 0x11u:
                ORA(instruction->second.mode);
                break;

            case 0x48u:
                PHA(instruction->second.mode);
                break;
            case 0x08u:
                PHP(instruction->second.mode);
                break;
            case 0x68u:
                PLA(instruction->second.mode);
                break;
            case 0x28u:
                PLP(instruction->second.mode);
                break;

            case 0x2Au:
                ROL(instruction->second.mode);
                break;
            case 0x26u:
                ROL(instruction->second.mode);
                break;
            case 0x36u:
                ROL(instruction->second.mode);
                break;
            case 0x2Eu:
                ROL(instruction->second.mode);
                break;
            case 0x3Eu:
                ROL(instruction->second.mode);
                break;

            case 0x6Au:
                ROR(instruction->second.mode);
                break;
            case 0x66u:
                ROR(instruction->second.mode);
                break;
            case 0x76u:
                ROR(instruction->second.mode);
                break;
            case 0x6Eu:
                ROR(instruction->second.mode);
                break;
            case 0x7Eu:
                ROR(instruction->second.mode);
                break;

            case 0x40u:
                RTI(instruction->second.mode);
                break;
            case 0x60u:
                RTS(instruction->second.mode);
                break;

            case 0xE9u:
                SBC(instruction->second.mode);
                break;
            case 0xE5u:
                SBC(instruction->second.mode);
                break;
            case 0xF5u:
                SBC(instruction->second.mode);
                break;
            case 0xEDu:
                SBC(instruction->second.mode);
                break;
            case 0xFDu:
                SBC(instruction->second.mode);
                break;
            case 0xF9u:
                SBC(instruction->second.mode);
                break;
            case 0xE1u:
                SBC(instruction->second.mode);
                break;
            case 0xF1u:
                SBC(instruction->second.mode);
                break;

            case 0x38u:
                SEC(instruction->second.mode);
                break;
            case 0xF8u:
                SED(instruction->second.mode);
                break;
            case 0x78u:
                SEI(instruction->second.mode);
                break;

            case 0x85u:
                STA(instruction->second.mode);
                break;
            case 0x95u:
                STA(instruction->second.mode);
                break;
            case 0x8Du:
                STA(instruction->second.mode);
                break;
            case 0x9Du:
                STA(instruction->second.mode);
                break;
            case 0x99u:
                STA(instruction->second.mode);
                break;
            case 0x81u:
                STA(instruction->second.mode);
                break;
            case 0x91u:
                STA(instruction->second.mode);
                break;

            case 0x86u:
                STX(instruction->second.mode);
                break;
            case 0x96u:
                STX(instruction->second.mode);
                break;
            case 0x8Eu:
                STX(instruction->second.mode);
                break;

            case 0x84u:
                STY(instruction->second.mode);
                break;
            case 0x94u:
                STY(instruction->second.mode);
                break;
            case 0x8Cu:
                STY(instruction->second.mode);
                break;

            case 0xAAu:
                TAX(instruction->second.mode);
                break;
            case 0xA8u:
                TAY(instruction->second.mode);
                break;
            case 0xBAu:
                TSX(instruction->second.mode);
                break;
            case 0x8Au:
                TXA(instruction->second.mode);
                break;
            case 0x9Au:
                TXS(instruction->second.mode);
                break;
            case 0x98u:
                TYA(instruction->second.mode);
                break;


            case 0x04:
                case 0x44:
                case 0x64:
                case 0x14:
                case 0x34:
                case 0x54:
                case 0x74:
                case 0xd4:
                case 0xf4:
                case 0x0c:
                case 0x1c:
                case 0x3c:
                case 0x5c:
                case 0x7c:
                case 0xdc:
                case 0xfc:
                    // TODO
                    break;
            default:
                assert(false);
            }
            if (program_counter_state == program_counter_) {
                program_counter_ += instruction->second.len - 1;
            }
            cycles_ += instruction->second.cycles;
            callback(this);
        }

    }
}

void CPU::ALR(AddressingMode mode)
{
    AND(mode);
    LSR(AddressingMode::Accumulator);
}

void CPU::ANC(AddressingMode mode)
{
    AND(mode);
    status_.is_set(Flag::NEGATIVE) ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
}

void CPU::ARR(AddressingMode mode)
{
    AND(mode);

    const bool old_carry = status_.is_set(Flag::CARRY);
    register_a_ >>= 1;
    if (old_carry)
        register_a_ |= 0x80u;

    update_zero(register_a_);
    update_negative(register_a_);
    (register_a_ & 0x40u) != 0 ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    ((register_a_ & 0x40u) >> 1) != (register_a_ & 0x20u)
        ? status_.set(Flag::OVERFLOWED)
        : status_.remove(Flag::OVERFLOWED);
}

void CPU::AXS(AddressingMode mode)
{
    const uint8_t value = bus.read(get_operand_address(mode));
    const uint8_t source = register_a_ & register_x_;
    const uint8_t result = static_cast<uint8_t>(source - value);

    source >= value ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    register_x_ = result;
    update_zero(register_x_);
    update_negative(register_x_);
}

void CPU::DCP(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    const uint8_t value = static_cast<uint8_t>(bus.read(address) - 1);
    bus.write(address, value);

    const uint8_t result = static_cast<uint8_t>(register_a_ - value);
    update_zero(result);
    update_negative(result);
    register_a_ >= value ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
}

void CPU::ISB(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    const uint8_t value = static_cast<uint8_t>(bus.read(address) + 1);
    bus.write(address, value);

    add_to_register_a(static_cast<uint8_t>(-value - 1));
}

void CPU::LAX(AddressingMode mode)
{
    const uint8_t value = bus.read(get_operand_address(mode));
    register_a_ = value;
    register_x_ = value;
    update_zero(value);
    update_negative(value);
}

void CPU::RLA(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    uint8_t value = bus.read(address);
    const bool old_carry = status_.is_set(Flag::CARRY);

    (value & 0x80u) != 0 ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    value = static_cast<uint8_t>(value << 1);
    if (old_carry)
        value |= 1u;
    bus.write(address, value);

    register_a_ &= value;
    update_zero(register_a_);
    update_negative(register_a_);
}

void CPU::RRA(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    uint8_t value = bus.read(address);
    const bool old_carry = status_.is_set(Flag::CARRY);

    (value & 1u) != 0 ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    value >>= 1;
    if (old_carry)
        value |= 0x80u;
    bus.write(address, value);

    add_to_register_a(value);
}

void CPU::SAX(AddressingMode mode)
{
    bus.write(get_operand_address(mode), static_cast<uint8_t>(register_a_ & register_x_));
}

void CPU::SHX(AddressingMode mode)
{
    const uint16_t base = bus.read_u16(program_counter_);
    const uint16_t address = get_operand_address(mode);
    const uint8_t value = static_cast<uint8_t>(register_x_ & (((base >> 8) + 1u) & 0xffu));
    bus.write(address, value);
}

void CPU::SHY(AddressingMode mode)
{
    const uint16_t base = bus.read_u16(program_counter_);
    const uint16_t address = get_operand_address(mode);
    const uint8_t value = static_cast<uint8_t>(register_y_ & (((base >> 8) + 1u) & 0xffu));
    bus.write(address, value);
}

void CPU::SLO(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    uint8_t value = bus.read(address);

    (value & 0x80u) != 0 ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    value = static_cast<uint8_t>(value << 1);
    bus.write(address, value);

    register_a_ |= value;
    update_zero(register_a_);
    update_negative(register_a_);
}

void CPU::SRE(AddressingMode mode)
{
    const uint16_t address = get_operand_address(mode);
    uint8_t value = bus.read(address);

    (value & 1u) != 0 ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);
    value >>= 1;
    bus.write(address, value);

    register_a_ ^= value;
    update_zero(register_a_);
    update_negative(register_a_);
}

void CPU::trace_instruction(uint16_t pc, const OpCode& instruction)
{
    if (logger_ == nullptr) {
        return;
    }

    const uint8_t operand = instruction.len > 1
        ? bus.read(static_cast<uint16_t>(pc + 1))
        : 0;
    const uint16_t operand_u16 = instruction.len > 2
        ? static_cast<uint16_t>(operand | (bus.read(static_cast<uint16_t>(pc + 2)) << 8))
        : 0;

    std::ostringstream asm_text;
    asm_text << instruction.mnemonic;

    switch (instruction.mode) {
    case Accumulator:
        asm_text << " A";
        break;
    case Immediate:
        asm_text << " #$" << hex_u8(operand);
        break;
    case ZeroPage:
        asm_text << " $" << hex_u8(operand)
                 << " = " << trace_u8(bus, operand);
        break;
    case ZeroPage_X:
    {
        const uint8_t address = static_cast<uint8_t>(operand + register_x_);
        asm_text << " $" << hex_u8(operand) << ",X @ " << hex_u8(address)
                 << " = " << trace_u8(bus, address);
        break;
    }
    case ZeroPage_Y:
    {
        const uint8_t address = static_cast<uint8_t>(operand + register_y_);
        asm_text << " $" << hex_u8(operand) << ",Y @ " << hex_u8(address)
                 << " = " << trace_u8(bus, address);
        break;
    }
    case Absolute:
        if (instruction.mnemonic == "JMP" || instruction.mnemonic == "JSR") {
            asm_text << " $" << hex_u16(operand_u16);
        } else {
            asm_text << " $" << hex_u16(operand_u16)
                     << " = " << trace_u8(bus, operand_u16);
        }
        break;
    case Absolute_X:
    {
        const uint16_t address = static_cast<uint16_t>(operand_u16 + register_x_);
        asm_text << " $" << hex_u16(operand_u16) << ",X @ " << hex_u16(address)
                 << " = " << trace_u8(bus, address);
        break;
    }
    case Absolute_Y:
    {
        const uint16_t address = static_cast<uint16_t>(operand_u16 + register_y_);
        asm_text << " $" << hex_u16(operand_u16) << ",Y @ " << hex_u16(address)
                 << " = " << trace_u8(bus, address);
        break;
    }
    case Indirect:
    {
        const uint16_t address = (operand_u16 & 0x00ffu) == 0x00ffu
            ? static_cast<uint16_t>(
                bus.read(operand_u16) |
                (bus.read(static_cast<uint16_t>(operand_u16 & 0xff00u)) << 8)
            )
            : bus.read_u16(operand_u16);
        asm_text << " ($" << hex_u16(operand_u16) << ") = " << hex_u16(address);
        break;
    }
    case Indirect_X:
    {
        const uint8_t pointer = static_cast<uint8_t>(operand + register_x_);
        const uint16_t address = static_cast<uint16_t>(
            bus.read(pointer) | (bus.read(static_cast<uint8_t>(pointer + 1)) << 8)
        );
        asm_text << " ($" << hex_u8(operand) << ",X) @ " << hex_u8(pointer)
                 << " = " << hex_u16(address) << " = " << trace_u8(bus, address);
        break;
    }
    case Indirect_Y:
    {
        const uint16_t base = static_cast<uint16_t>(
            bus.read(operand) | (bus.read(static_cast<uint8_t>(operand + 1)) << 8)
        );
        const uint16_t address = static_cast<uint16_t>(base + register_y_);
        asm_text << " ($" << hex_u8(operand) << "),Y = " << hex_u16(base)
                 << " @ " << hex_u16(address) << " = " << trace_u8(bus, address);
        break;
    }
    case NoneAddressing:
        if (instruction.len == 2 && !instruction.mnemonic.empty() &&
            instruction.mnemonic[0] == 'B') {
            const auto offset = static_cast<int8_t>(operand);
            const uint16_t target = static_cast<uint16_t>(pc + instruction.len + offset);
            asm_text << " $" << hex_u16(target);
        }
        break;
    }

    const uint64_t ppu_ticks = cycles_ * 3;
    const uint64_t ppu_scanline = (ppu_ticks / 341) % 262;
    const uint64_t ppu_cycle = ppu_ticks % 341;

    std::ostringstream line;
    line << hex_u16(pc) << "  "
         << instruction_bytes(bus, pc, instruction.len) << "  "
         << std::left << std::setw(32) << asm_text.str()
         << std::right
         << " A:" << hex_u8(register_a_)
         << " X:" << hex_u8(register_x_)
         << " Y:" << hex_u8(register_y_)
         << " P:" << hex_u8(status_.status)
         << " SP:" << hex_u8(stack_pointer_)
         << " PPU:" << std::dec << std::setw(3) << ppu_scanline
         << "," << std::setw(3) << ppu_cycle
         << " CYC:" << cycles_;

    logger_->log(line.str());
}

uint16_t CPU::get_operand_address(AddressingMode mode)
{
    switch (mode)
    {
    case AddressingMode::Immediate:
        return program_counter_;
    case AddressingMode::ZeroPage:
        return bus.read(program_counter_);
    case AddressingMode::ZeroPage_X:
        return static_cast<uint8_t>(bus.read(program_counter_) + register_x_);
    case AddressingMode::ZeroPage_Y:
        return static_cast<uint8_t>(bus.read(program_counter_) + register_y_);
    case AddressingMode::Absolute:
    {
        return bus.read_u16(program_counter_);
    }
    case AddressingMode::Absolute_X:
    {
        uint16_t base = bus.read_u16(program_counter_);
        return base + register_x_;
    }
    case AddressingMode::Absolute_Y:
    {
        uint16_t base = bus.read_u16(program_counter_);
        return base + register_y_;
    }
    case AddressingMode::Indirect_X:
    {
        const uint8_t ptr = static_cast<uint8_t>(bus.read(program_counter_) + register_x_);
        const uint8_t lo = bus.read(ptr);
        const uint8_t hi = bus.read(static_cast<uint8_t>(ptr + 1));

        return static_cast<uint16_t>(hi << 8) | static_cast<uint16_t>(lo);
    }

    case AddressingMode::Indirect_Y:
    {
        const uint8_t ptr = bus.read(program_counter_);
        const uint8_t lo = bus.read(ptr);
        const uint8_t hi = bus.read(static_cast<uint8_t>(ptr + 1));
        const uint16_t base = static_cast<uint16_t>(hi << 8) | static_cast<uint16_t>(lo);

        return static_cast<uint16_t>(base + register_y_);
    }
    default:
        break;
    }
    return 0;
}

void CPU::load(std::vector<uint8_t> program)
{
    for (size_t i = 0; i < program.size(); i++)
    {
        bus.write(static_cast<uint16_t>(0x0600 + i), program[i]);
    }
    program_counter_ = 0x0600;
}

// Utils

BitFlags &CPU::update_zero(uint8_t result)
{
    return result == 0 ? status_.set(Flag::ZERO) : status_.remove(Flag::ZERO);
}

BitFlags &CPU::update_negative(uint8_t result)
{
    return (result & Flag::NEGATIVE) != 0 ? status_.set(Flag::NEGATIVE) : status_.remove(Flag::NEGATIVE);
}

void CPU::branch_if(bool condition)
{
    if (condition)
    {
        const auto offset = static_cast<int8_t>(bus.read(program_counter_));
        program_counter_ = static_cast<uint16_t>(program_counter_ + 1 + offset);
    }
}

void CPU::add_to_register_a(uint8_t value)
{
    const auto sum = static_cast<int>(register_a_) + static_cast<int>(value) + (status_.is_set(Flag::CARRY) ? 1 : 0);
    const bool carry = sum > 0xff;
    // Set carry
    carry ? status_.set(Flag::CARRY) : status_.remove(Flag::CARRY);

    const auto result = static_cast<uint8_t>(sum);
    // Set overflow
    ((value ^ result) & (result ^ register_a_) & 0x80) != 0
        ? status_.set(Flag::OVERFLOWED)
        : status_.remove(Flag::OVERFLOWED);
    register_a_ = result;
    update_negative(register_a_);
    update_zero(register_a_);
}

void CPU::set_register_a(uint8_t data)
{
    register_a_ = data;
    update_zero(register_a_);
    update_negative(register_a_);
}

/**
 * ADC - Add with Carry
 * A,Z,C,N = A+M+C
 * This instruction adds the contents of a memory location to the accumulator together with the carry bit. If overflow occurs the carry bit is set, this enables multiple byte addition to be performed.
 * Processor Status after use:
 * C | If overflow in bit 7
 * Z | A == 0 ?
 * V | If sign bit is incorrect
 * N | If bit 7 set
 */
void CPU::ADC(AddressingMode mode)
{
    auto address = get_operand_address(mode);
    auto value = bus.read(address);

    add_to_register_a(value);
}
