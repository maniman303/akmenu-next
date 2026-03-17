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

cCalendarWnd::cCalendarWnd() : cForm(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, NULL, "calendar window") {}

cCalendarWnd::~cCalendarWnd() {}

void cCalendarWnd::init() {
    setEngine(GE_SUB);
    loadAppearance(SFN_UPPER_SCREEN_BG);
}

cWindow& cCalendarWnd::loadAppearance(const std::string& aFileName) {
    std::string aFileName4 = replaceInString(aFileName, ".bmp", "_cal4.bmp");
    std::string aFileName5 = replaceInString(aFileName, ".bmp", "_cal5.bmp");
    _background = createBMP15FromFile(aFileName);
    _background4 = createBMP15FromFile(aFileName4);
    _background5 = createBMP15FromFile(aFileName5);

    return *this;
}

static int weeksInCurrentMonth() {
    u8 daysOfMonth = datetime().daysOfMonth();
    u8 weekdDayOfMonthFirstDay = datetime().weekDayOfMonthFirstDay();

    return ((daysOfMonth + weekdDayOfMonthFirstDay - 1) / 7) + 1;
}

void cCalendarWnd::draw() {
    int weeks = weeksInCurrentMonth();
    if (weeks == 4 && _background4.valid()) {
        gdi().bitBlt(_background4.buffer(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, selectedEngine());
        return;
    } else if (weeks == 5 && _background5.valid()) {
        gdi().bitBlt(_background5.buffer(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, selectedEngine());
        return;
    } else {
        gdi().bitBlt(_background.buffer(), 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, selectedEngine());
    }
}
