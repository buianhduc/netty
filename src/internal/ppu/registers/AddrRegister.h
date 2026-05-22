//
// Created by Đức Bùi on 15/5/2026.
//
#pragma once
#include <cstdint>
#include <utility>

class AddrRegister {
    std::pair<uint8_t, uint8_t> value;
    bool hi_ptr;
    public:
    AddrRegister(): value(0,0), hi_ptr(true) {};
    void set_value(uint16_t value);
    void update(uint8_t value);
    void update(uint8_t value, bool first_write);
    void increment(uint8_t inc);
    void reset_latch();
    [[nodiscard]] uint16_t get() const;
};
