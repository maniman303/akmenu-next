#pragma once

#include <nds.h>
#include "ui/point.h"
#include "singleton.h"

class Twin {
  public:
    s32 int32(s32 start, s32 end, u16 value);
    s16 int16(s16 start, s16 end, u16 value);
    cPoint point(cPoint start, cPoint end, u16 value);
};

typedef t_singleton<Twin> Twin_s;
inline Twin& twin() {
    return Twin_s::instance();
}