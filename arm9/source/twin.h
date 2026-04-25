#pragma once

#include <nds.h>
#include "ui/point.h"
#include "singleton.h"

enum class TWIN_EASE : u8 {
    LINEAR,
    EASE_OUT
};

class Twin {
  public:
    s32 int32(s32 start, s32 end, u16 value, TWIN_EASE ease = TWIN_EASE::LINEAR);
    s16 int16(s16 start, s16 end, u16 value, TWIN_EASE ease = TWIN_EASE::LINEAR);
    cPoint point(cPoint start, cPoint end, u16 value, TWIN_EASE ease = TWIN_EASE::LINEAR);
  
  private:
    u16 applyEase(u16 value, TWIN_EASE ease);
};

typedef t_singleton<Twin> Twin_s;
inline Twin& twin() {
    return Twin_s::instance();
}