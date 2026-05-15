#include "bus.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

void Bus::write(const uint16_t address, const uint8_t data) {

    if (std::clamp(address, RAM, RAM_MIRROR_END)==address){
        // Only get the first 11 bits
        auto mirror_down_address = address % 0x0800;
        cpu_vram_[mirror_down_address] = data;
        return;
    }
    if (std::clamp(address, PPU_REGISTERS, PPU_REGISTERS_MIRROR_END)==address) {
        auto _mirror_down_addr = address & 0b0010000000000111;
        // TODO: implement PPU
        assert(false);
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4000u),
        static_cast<uint16_t>(0x401fu)) == address) {
        // TODO: implement APU and I/O registers.
        return;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x8000u),
       static_cast<uint16_t>(0xffffu))== address) {
            throw std::runtime_error("Attempting to write to ROM");

    }
    throw std::invalid_argument("Address out of range trying to write: " + std::to_string(address));
}

void Bus::write_u16(const uint16_t address, uint16_t data)
{
    const uint8_t hi = data >> 8;
    const uint8_t lo = data & 0xffu;
    write(address, lo);
    write(address+1, hi);
}

uint8_t Bus::read(uint16_t address) const {
    if (std::clamp(address, RAM, RAM_MIRROR_END) == address){
        // Only get the first 11 bits
        auto mirror_down_address = address % 0x0800;
        return cpu_vram_[mirror_down_address];
    }
    if (std::clamp(address, PPU_REGISTERS, PPU_REGISTERS_MIRROR_END) == address) {
        auto _mirror_down_addr = address & 0b0010000000000111;
        // TODO: implement PPU
        assert(false);
    }
    if (std::clamp(address, static_cast<uint16_t>(0x4000u),
        static_cast<uint16_t>(0x401fu)) == address) {
        // TODO: implement APU and I/O registers.
        return 0xff;
    }
    if (std::clamp(address, static_cast<uint16_t>(0x8000u),
        static_cast<uint16_t>(0xffffu)) == address) {
        return read_prg_rom(address);
    }
    throw std::invalid_argument("Address out of range trying to read: " + std::to_string(address));
}

uint16_t Bus::read_u16(uint16_t address) const {
    const uint16_t low = read(address);
    const uint16_t high = read(address + 1);
    return (high << 8 | low);
}
