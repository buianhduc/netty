//
// Created by Đức Bùi on 19/5/2026.
//

#include "ScrollRegister.h"

void ScrollRegister::update(uint8_t data) {
    update(data, x_ptr_);
    x_ptr_ = !x_ptr_;
}

void ScrollRegister::update(uint8_t data, bool first_write) {
    if (first_write) {
        x_scroll_ = data;
    } else {
        y_scroll_ = data;
    }
}

void ScrollRegister::reset_latch() {
    x_ptr_ = true;
}

uint8_t ScrollRegister::x_scroll() const {
    return x_scroll_;
}

uint8_t ScrollRegister::y_scroll() const {
    return y_scroll_;
}
