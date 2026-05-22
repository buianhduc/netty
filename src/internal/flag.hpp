#pragma once
#include <cstdint>
/// # Status Register (P) http://wiki.nesdev.com/w/index.php/Status_flags
///
///  7 6 5 4 3 2 1 0
///  N V _ B D I Z C
///  | |   | | | | +--- Carry Flag
///  | |   | | | +----- Zero Flag
///  | |   | | +------- Interrupt Disable
///  | |   | +--------- Decimal Mode (not used on NES)
///  | |   +----------- Break Command
///  | +--------------- Overflow Flag
///  +----------------- Negative Flag
///
enum Flag {

    CARRY = (1 << 0),
    ZERO = (1 << 1),
    INTERRUPT = (1 << 2),
    // UNUSED 
    DECIMAL = (1 << 3),
    BREAK = (1 << 4),
    UNUSED = (1 << 5),
    // END OF UNUSED
    OVERFLOWED = (1 << 6),
    NEGATIVE = (1 << 7)
};

class BitFlags {
    

    public:
        uint8_t status = 0;
        BitFlags(uint8_t initialValues)
        {
            status = initialValues;
        }
        BitFlags& set(uint8_t flag) {
            status |= flag;
            return *this;
        }
        BitFlags& remove(uint8_t flag) {
            status &= static_cast<uint8_t>(~flag);
            return *this;
        }
        [[nodiscard]] bool is_set(uint8_t flag) const {
            return (status & flag);
        }
};
