#pragma once

#include <vector>
#include <functional>
#include "bus.hpp"
#include "addressing_mode.hpp"
#include "flag.hpp"
#include "logger.hpp"
#include "opcodes.hpp"



const uint16_t STACK = 0x0100;
const uint8_t STACK_RESET = 0xfd;

class CPU {
    public:
        CPU(const ROM &rom): bus(rom) {};
        CPU(): bus(Bus()) {};
        ~CPU();
        void interpret(std::vector<uint8_t> program);
        void interpret_with_callback(const std::function<void(CPU *)> &callback);
        void load_and_run(std::vector<uint8_t> program);
        void mem_write(uint16_t address, uint8_t data);
        uint8_t register_a() const;
        uint8_t register_x() const;
        uint8_t status() const;
        void set_register_a_for_test(uint8_t data);
        void set_register_x_for_test(uint8_t data);
        Bus bus;
        void reset();
        void set_logger(Logger* logger);

    private:
        uint8_t register_a_ = 0;
        uint8_t register_x_ = 0;
        uint8_t register_y_ = 0;
        BitFlags status_ = BitFlags(0b00100100);
        uint16_t program_counter_ = 0x8000;
        uint8_t stack_pointer_ = STACK_RESET;
        uint64_t cycles_ = 0;
        Logger* logger_ = nullptr;

        
        uint16_t get_operand_address(AddressingMode mode);
        void load (std::vector<uint8_t> program);

        void ADC(AddressingMode mode);
        void AND(AddressingMode mode);
        void ASL(AddressingMode mode);
        void BCC(AddressingMode mode);
        void BCS(AddressingMode mode);
        void BEQ(AddressingMode mode);
        void BIT(AddressingMode mode);
        void BMI(AddressingMode mode);
        void BNE(AddressingMode mode);
        void BPL(AddressingMode mode);
        void BRK(AddressingMode mode);
        void BVC(AddressingMode mode);
        void BVS(AddressingMode mode);
        void CLC(AddressingMode mode);
        void CLD(AddressingMode mode);
        void CLI(AddressingMode mode);
        void CLV(AddressingMode mode);
        void CMP(AddressingMode mode);
        void CPX(AddressingMode mode);
        void CPY(AddressingMode mode);
        void DEC(AddressingMode mode);
        void DEX(AddressingMode mode);
        void DEY(AddressingMode mode);
        void EOR(AddressingMode mode);
        void INC(AddressingMode mode);
        void INX(AddressingMode mode);
        void INY(AddressingMode mode);
        void JMP(AddressingMode mode);
        void JSR(AddressingMode mode);
        void LDA(AddressingMode mode);
        void LDX(AddressingMode mode);
        void LDY(AddressingMode mode);
        void LSR(AddressingMode mode);
        void NOP(AddressingMode mode);
        void ORA(AddressingMode mode);
        void PHA(AddressingMode mode);
        void PHP(AddressingMode mode);
        void PLA(AddressingMode mode);
        void PLP(AddressingMode mode);
        void ROL(AddressingMode mode);
        void ROR(AddressingMode mode);
        void RTI(AddressingMode mode);
        void RTS(AddressingMode mode);
        void SBC(AddressingMode mode);
        void SEC(AddressingMode mode);
        void SED(AddressingMode mode);
        void SEI(AddressingMode mode);
        void STA(AddressingMode mode);
        void STX(AddressingMode mode);
        void STY(AddressingMode mode);
        void TAX(AddressingMode mode);
        void TAY(AddressingMode mode);
        void TSX(AddressingMode mode);
        void TXA(AddressingMode mode);
        void TXS(AddressingMode mode);
        void TYA(AddressingMode mode);
        void ALR(AddressingMode mode);
        void ANC(AddressingMode mode);
        void ARR(AddressingMode mode);
        void AXS(AddressingMode mode);
        void DCP(AddressingMode mode);
        void ISB(AddressingMode mode);
        void LAX(AddressingMode mode);
        void RLA(AddressingMode mode);
        void RRA(AddressingMode mode);
        void SAX(AddressingMode mode);
        void SHX(AddressingMode mode);
        void SHY(AddressingMode mode);
        void SLO(AddressingMode mode);
        void SRE(AddressingMode mode);

        // Registers operations
        void add_to_register_a(uint8_t data);
        void set_register_a(uint8_t data);

        // Stack operations
        uint8_t stack_pop();
        void stack_push(uint8_t data);
        uint16_t stack_pop_u16();
        void stack_push_u16(uint16_t data);

        // Utils:
        BitFlags& update_zero(uint8_t result);
        BitFlags& update_negative(uint8_t result);
        void branch_if(bool condition);
        void trace_instruction(uint16_t pc, const OpCode& instruction);
};
