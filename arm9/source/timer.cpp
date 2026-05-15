/*
    timer.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "timer.h"
#include "divider.h"
#include "stringtool.h"

cTimer::cTimer() {
    initTimer();
}

void cTimer::initTimer() {
    _ticks = 0;
    _frames = 0;
    _fpsCounter = 0;
    _fps = 0;
}

u32 cTimer::updateTimer() {
    _ticks++;
    if (_ticks == UINT32_MAX) {
        _ticks = 0;
    }

    return _ticks;
}

u32 cTimer::updateFrames() {
    _frames++;
    if (_fpsCounter == 0) {
        _fpsCounter = _ticks;
        return _frames;
    }

    u32 diff = _ticks - _fpsCounter;
    _fpsCounter = _ticks;

    _fps = hw::divide(60, diff);

    return _frames;
}

u32 cTimer::getTick() const {
    return _ticks;
}

u32 cTimer::getFrame() const {
    return _frames;
}

u32 cTimer::getFps() const {
    return _fps;
}