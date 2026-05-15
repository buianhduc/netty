#pragma once

#include "addressing_mode.hpp"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

class OpCode {
    public:
        OpCode(
            uint8_t code,
            std::string mnemonic,
            uint8_t len,
            uint8_t cycles,
            AddressingMode mode
        );
        uint8_t code;
        std::string mnemonic;
        uint8_t len;
        uint8_t cycles;
        AddressingMode mode;
};

extern const std::vector<OpCode> OP_CODES;
extern const std::unordered_map<uint8_t, OpCode> InstructionTable;
