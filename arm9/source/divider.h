#pragma once

#include <nds.h>

namespace hw {
    s32 divide(s32 numerator, s32 denominator);
    s32 divide(s32 numerator, s32 denominator, s32& rest);
    s32 mod(s32 num, s32 den);
}