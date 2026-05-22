//
// Created by Đức Bùi on 19/5/2026.
//

#ifndef NES_EMULATOR_STATUSREGISTER_H
#define NES_EMULATOR_STATUSREGISTER_H
#include "PPURegister.h"

/**
*     // 7  bit  0
    // ---- ----
    // VSO. ....
    // |||| ||||
    // |||+-++++- Least significant bits previously written into a PPU register
    // |||        (due to register not being updated for this address)
    // ||+------- Sprite overflow. The intent was for this flag to be set
    // ||         whenever more than eight sprites appear on a scanline, but a
    // ||         hardware bug causes the actual behavior to be more complicated
    // ||         and generate false positives as well as false negatives; see
    // ||         PPU sprite evaluation. This flag is set during sprite
    // ||         evaluation and cleared at dot 1 (the second dot) of the
    // ||         pre-render line.
    // |+-------- Sprite 0 Hit.  Set when a nonzero pixel of sprite 0 overlaps
    // |          a nonzero background pixel; cleared at dot 1 of the pre-render
    // |          line.  Used for raster timing.
    // +--------- Vertical blank has started (0: not in vblank; 1: in vblank).
    //            Set at dot 1 of line 241 (the line *after* the post-render
    //            line); cleared after reading $2002 and at dot 1 of the
    //            pre-render line.
 */
enum StatusRegisterBit : uint8_t {
    NOTUSED = 1 << 0,
    NOTUSED1 = 1 << 1,
    NOTUSED2 = 1 << 2,
    NOTUSED3 = 1 << 3,
    NOTUSED4 = 1 << 4,
    SPRITE_OVERFLOW = 1 << 5,
    SPRITE_ZERO_HIT = 1 << 6,
    VBLANK_STARTED = 1 << 7,


};

class StatusRegister : PPURegister {
    public:
    StatusRegister() : PPURegister(true, false) {};
    uint8_t read(uint8_t open_bus);
    uint8_t peek(uint8_t open_bus) const;
    void set_vblank_started(bool value);
    void clear_vblank_started();
    void set_sprite_zero_hit(bool value);
    void clear_sprite_zero_hit();
    void set_sprite_overflow();
    void clear_sprite_overflow();
    void clear_rendering_flags();
    bool is_in_vblank() const;
};


#endif //NES_EMULATOR_STATUSREGISTER_H
