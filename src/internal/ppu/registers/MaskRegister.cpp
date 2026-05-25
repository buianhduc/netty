//
// Created by Đức Bùi on 19/5/2026.
//

#include "MaskRegister.h"

void MaskRegister::update(uint8_t data) {
    status = data;
}

bool MaskRegister::is_greyscale() {
    return is_set(MaskRegisterBit::GREYSCALE);
}

bool MaskRegister::leftmopst_8pxl_background() {
    return is_set(LEFTMOST_8PXL_BACKGROUND);
}

bool MaskRegister::leftmopst_8pxl_sprite()
{
    return is_set(LEFTMOST_8PXL_SPRITE);
}

bool MaskRegister::show_background() {
    return is_set(SHOW_BACKGROUND);
}

bool MaskRegister::show_sprite() const {
    return is_set(SHOW_SPRITE);
}

std::vector<Color> MaskRegister::emphasise() {
    std::vector<Color> emphasis;
    if (is_set(EMPHASISE_RED)) emphasis.push_back(ColorHelper::RED());
    if (is_set(EMPHASISE_GREEN)) emphasis.push_back(ColorHelper::GREEN());
    if (is_set(EMPHASISE_BLUE)) emphasis.push_back(ColorHelper::BLUE());

    return emphasis;
}
