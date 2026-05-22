#include "cpu.hpp"
// Stack operations
uint8_t CPU::stack_pop()
{
    stack_pointer_ += 1;
    return bus.read(static_cast<uint16_t>(STACK + stack_pointer_), true);
};
void CPU::stack_push(uint8_t data)
{
    bus.write(static_cast<uint16_t>(STACK + stack_pointer_), data, true);
    stack_pointer_ -= 1;
};
uint16_t CPU::stack_pop_u16()
{
    uint16_t lo = static_cast<uint16_t>(stack_pop());
    uint16_t hi = static_cast<uint16_t>(stack_pop());
    return static_cast<uint16_t>(hi << 8 | lo);
};
void CPU::stack_push_u16(uint16_t data)
{
    uint8_t hi = static_cast<uint8_t>(data >> 8);
    uint8_t lo = static_cast<uint8_t>(data & 0xffu);

    stack_push(hi);
    stack_push(lo);
};