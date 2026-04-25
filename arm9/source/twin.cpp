#include "twin.h"

u16 Twin::applyEase(u16 value, TWIN_EASE ease) {
    switch (ease) {
        case TWIN_EASE::EASE_OUT:
            return value * (200 - value) / 100;
        case TWIN_EASE::LINEAR:
        default:
            return value;
    }
}

s32 Twin::int32(s32 start, s32 end, u16 value, TWIN_EASE ease) {
    if (value == 0) {
        return start;
    }

    if (value >= 100) {
        return end;
    }
    
    value = applyEase(value, ease);
    return start + ((end - start) * (s32)value / (s32)100);
}

s16 Twin::int16(s16 start, s16 end, u16 value, TWIN_EASE ease) {
    if (value == 0) {
        return start;
    }

    if (value >= 100) {
        return end;
    }

    value = applyEase(value, ease);
    return start + ((end - start) * (s16)value / (s16)100);
}

cPoint Twin::point(cPoint start, cPoint end, u16 value, TWIN_EASE ease) {
    if (value == 0) {
        return start;
    }

    if (value >= 100) {
        return end;
    }

    value = applyEase(value, ease);
    s32 x = start.x + ((end.x - start.x) * (s32)value / (s32)100);
    s32 y = start.y + ((end.y - start.y) * (s32)value / (s32)100);

    return cPoint(x, y);
}