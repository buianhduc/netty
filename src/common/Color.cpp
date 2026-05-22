//
// Created by Đức Bùi on 19/5/2026.
//

#include "Color.h"

namespace ColorHelper {
    Color RED() {
        return {255, 0, 0};
    }

    Color GREEN() {
        return {0, 255, 0};
    }

    Color BLUE() {
        return {0, 0, 255};
    }

    Color color_rgb(uint8_t color_index) {
        switch (color_index)
        {
            case 0:
                return {0, 0, 0};
            case 1:
                return {UINT8_MAX, UINT8_MAX, UINT8_MAX};
            case 2:
            case 9:
                return {128, 128, 128};
            case 3:
            case 10:
                return {UINT8_MAX, 0, 0};
            case 4:
            case 11:
                return {0, UINT8_MAX, 0};
            case 5:
            case 12:
                return {0, 0, UINT8_MAX};
            case 6:
            case 13:
                return {UINT8_MAX, 0, UINT8_MAX};
            case 7:
            case 14:
                return {UINT8_MAX, UINT8_MAX, 0};
            default:
                return {0, UINT8_MAX, UINT8_MAX};
        }
    }
} // namespace ColorHelper
