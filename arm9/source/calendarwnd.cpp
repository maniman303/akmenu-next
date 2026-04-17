/*
    calendarwnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "calendarwnd.h"
#include "bmp15.h"
#include "gdi.h"
#include "globalsettings.h"
#include "systemfilenames.h"
#include "stringtool.h"
#include "datetime.h"

using namespace akui;

cCalendarWnd::cCalendarWnd() : cForm(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, "calendar window") {
    _filename = "";
    _lastDate = 0;
}

cCalendarWnd::~cCalendarWnd() {}

void cCalendarWnd::init() {
    setEngine(GE_SUB);
    loadAppearance(SFN_UPPER_SCREEN_BG);
}

cWindow& cCalendarWnd::loadAppearance(const std::string& aFileName) {
    _filename = aFileName;
    setRelativePosition(cPoint(0, 0));
    setSize(cSize(SCREEN_WIDTH, SCREEN_HEIGHT));

    return *this;
}

static int weeksInCurrentMonth() {
    u8 daysOfMonth = datetime().daysOfMonth();
    u8 weekdDayOfMonthFirstDay = datetime().weekDayOfMonthFirstDay();

    return ((daysOfMonth + weekdDayOfMonthFirstDay - 1) / 7) + 1;
}

void cCalendarWnd::drawBackdrop() {
    int weeks = weeksInCurrentMonth();
    std::string bgFile(_filename);
    if (weeks == 4) {
        bgFile = replaceInString(bgFile, ".bmp", "_cal4.bmp");
    } else if (weeks == 5) {
        bgFile = replaceInString(bgFile, ".bmp", "_cal5.bmp");
    }

    if (!_background.valid() || _background.filename() != bgFile) {
        destroyBMP15(_background);
        _background = createBMP15FromFile(bgFile);
    }

    if (!_background.valid()) {
        _background = createBMP15FromFile(_filename);
    }

    if (_background.width() < SCREEN_WIDTH || _background.height() < SCREEN_HEIGHT) {
        return;
    }

    _lastDate = datetime().now();

    gdi().bitBlt(_background.buffer(), position().x, position().y, size().x, size().y, selectedEngine());
}

bool cCalendarWnd::shouldRenderBackdrop() {
    u32 now = datetime().now();
    if (now == _lastDate && _background.valid()) {
        return false;
    }

    return true;
}
