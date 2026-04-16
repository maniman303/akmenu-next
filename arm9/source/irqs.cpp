/*
    irqs.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "irqs.h"
#include "analogclock.h"
#include "bigclock.h"
#include "calendar.h"
#include "calendarwnd.h"
#include "batterymeter.h"
#include "booticon.h"
#include "smalldate.h"
#include "smallclock.h"
#include "dbgtool.h"
#include "diskicon.h"
#include "timer.h"
#include "userinput.h"
#include "userwnd.h"
#include "fpscounter.h"
#include "logger.h"
#include "windowmanager.h"

using namespace akui;

bool cIRQ::_vblankStarted(false);

void cIRQ::init() {
    irqSet(IRQ_VBLANK, vBlank);
    irqSet(IRQ_CARD_LINE, cardMC);
}

void cIRQ::cardMC() {
    dbg_printf("cardMC\n");
    diskIcon().blink();
    REG_IF &= ~IRQ_CARD_LINE;
}

void cIRQ::vblankStart() {
    _vblankStarted = true;
}

void cIRQ::vblankStop() {
    _vblankStarted = false;
}

bool cIRQ::isVblankStarted() {
    return _vblankStarted;
}

void cIRQ::drawTop() {
    calendarWnd().draw();
    calendar().draw();
    analogClock().draw();
    bigClock().draw();
    batteryMeter().draw();
    bootIcon().draw();
    smallDate().draw();
    smallClock().draw();
    userWindow().draw();
    fpsCounter().draw();
}

void cIRQ::vBlank() {
    if (!_vblankStarted) return;

    timer().updateTimer();

    static u32 vBlankCounter = 0;

    if (vBlankCounter++ >= 29) {
        vBlankCounter = 0;
        bigClock().blinkColon();
        smallClock().flipColon();
        batteryMeter().flipIcon();
    }

    if (REG_ROMCTRL & CARD_BUSY)
        diskIcon().turnOn();
    else
        diskIcon().turnOff();
}
