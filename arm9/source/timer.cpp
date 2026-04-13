/*
    timer.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "timer.h"

cTimer::cTimer() {
    // initTimer();
}

void cTimer::initTimer() {
    _ticks = 0;
    _frames = 0;
    _fps = 0.f;
    _fpsCounter = 0;
}

u32 cTimer::updateTimer() {
    _ticks++;
    if (_ticks == UINT32_MAX) {
        _ticks = 0;
    }

    return _ticks;
}

u32 cTimer::updateFrame() {
    _frames++;

    return _frames;
}

u32 cTimer::getTick() {
    return _ticks;
}

u32 cTimer::getFrame() {
    return _frames;
}
