/*
    timer.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include "singleton.h"

class cTimer {
  public:
    cTimer();

    ~cTimer() {}

  public:
    void initTimer();

    u32 updateTimer();

    u32 updateFrames();

    u32 getTick() const;

    u32 getFrame() const;

    u32 getFps() const;

  private:
    u16 _prevFps;
    u16 _fps;
    u32 _fpsCounter;
    u32 _ticks;
    u32 _frames;
};

typedef t_singleton<cTimer> timer_s;
inline cTimer& timer() {
    return timer_s::instance();
}
