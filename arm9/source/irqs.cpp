/*
    irqs.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "irqs.h"
#include "dbgtool.h"
#include "timer.h"
#include "diskicon.h"
#include "logger.h"
#include "subwindowmanager.h"

bool cIRQ::_vblankStarted(false);

bool cIRQ::_presentScheduled(false);

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

void cIRQ::schedulePresent() {
    _presentScheduled = true;
}

void cIRQ::vBlank() {
    if (!_vblankStarted) return;

    timer().updateTimer();

    static u32 vBlankCounter = 0;

    if (vBlankCounter++ >= 29) {
        vBlankCounter = 0;
        subWindowManager().blink();
    }

    if (REG_ROMCTRL & CARD_BUSY)
        diskIcon().turnOn();
    else
        diskIcon().turnOff();

    if (_presentScheduled) {
        _presentScheduled = false;
        gdi().present();
    }
}
