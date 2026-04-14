/*
    timer.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "timer.h"

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

    u16 fps = 60 / diff;
    if (_fps == _prevFps && fps != _fps && fps != _prevFps) {
        _prevFps = fps;
        return _frames;
    }

    _prevFps = _fps;
    _fps = fps;

    return _frames;
}

u32 cTimer::getTick() {
    return _ticks;
}

u32 cTimer::getFrame() {
    return _frames;
}

u32 cTimer::getFps() {
    return _fps;
}