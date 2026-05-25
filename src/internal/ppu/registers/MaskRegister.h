//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_MASKREGISTER_H
#define NES_EMULATOR_MASKREGISTER_H
#include <cstdint>
#include <vector>

#include "PPURegister.h"
#include "common/Color.h"


/**
* 7  bit  0
---- ----
BGRs bMmG
|||| ||||
|||| |||+- Greyscale (0: normal color, 1: greyscale)
|||| ||+-- 1: Show background in leftmost 8 pixels of screen, 0: Hide
|||| |+--- 1: Show sprites in leftmost 8 pixels of screen, 0: Hide
|||| +---- 1: Enable background rendering
|||+------ 1: Enable sprite rendering
||+------- Emphasize red (green on PAL/Dendy)
|+-------- Emphasize green (red on PAL/Dendy)
+--------- Emphasize blue

 */

enum MaskRegisterBit {
    GREYSCALE = 1 << 0,
    LEFTMOST_8PXL_BACKGROUND = 1 << 1,
    LEFTMOST_8PXL_SPRITE = 1 << 2,
    SHOW_BACKGROUND = 1 << 3,
    SHOW_SPRITE = 1 << 4,
    EMPHASISE_RED = 1 << 5,
    EMPHASISE_GREEN = 1 << 6,
    EMPHASISE_BLUE = 1 << 7,
};
class MaskRegister : PPURegister{
    public:
        explicit MaskRegister(const uint8_t init_value = 0) : PPURegister(false, true,
                                                                          init_value) {};
        ~MaskRegister() = default;

        void update(uint8_t data);
        bool is_greyscale();
        bool leftmopst_8pxl_background();
        bool leftmopst_8pxl_sprite();
        bool show_background();
        bool show_sprite() const;
        std::vector<Color> emphasise();

};


#endif //NES_EMULATOR_MASKREGISTER_H
