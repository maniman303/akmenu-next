/*
    timer.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "timer.h"

vu64 cTimer::_overFlow = 0;
cTimer::cTimer() {
    // initTimer();
}

void cTimer::timerInterruptHandler() {
    _overFlow += 65536;
}

void cTimer::initTimer() {
    _lastTime = 0;
    _currentTime = 0;
    _overFlow = 0;
    _fps = 0.f;
    _fpsCounter = 0;
    irqEnable(IRQ_TIMER0);
    irqSet(IRQ_TIMER0, cTimer::timerInterruptHandler);
    TIMER0_DATA = 0;  // set reload value
    TIMER0_CR = TIMER_ENABLE | TIMER_DIV_1 | TIMER_IRQ_REQ;
}

double cTimer::updateTimer() {
    _ticks++;
    if (_ticks == UINT32_MAX) {
        _ticks = 0;
    }

    _currentTime = (_overFlow + TIMER0_DATA) * _factor;
    return _currentTime;
}

double cTimer::updateFps() {
    if (_fpsCounter++ > 60) {
        double elapsedTime = _currentTime - _lastTime;
        _fps = _fpsCounter / elapsedTime;
        _fpsCounter = 0;
        _lastTime = _currentTime;
    }
    return _fps;
}

double cTimer::getTime() {
    return _currentTime;
}

vu64 cTimer::getTick() {
    return _ticks;
}

double cTimer::getFps() {
    return _fps;
}
