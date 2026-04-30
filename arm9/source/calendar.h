/*
    calendar.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "bmp15.h"
#include "datetime.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

class cCalendar : public cWindow {
  public:
    cCalendar();
    ~cCalendar() {}

  public:
    void init();
    void drawBackdrop() override;
    bool shouldRenderBackdrop() override;
    void onRenderBackdrop() override;

  protected:
    void drawDayNumber(u8 day, u8 weekdDayOfMonthFirstDay, u8 today);
    void drawText(const cPoint& position, u32 value, u32 factor);
    void drawNumber(const cPoint& position, u32 index, u32 value);

    cPoint _dayPosition;
    cSize _daySize;
    cPoint _dayxPosition;
    cPoint _monthPosition;
    cPoint _yearPosition;
    COLOR _dayHighlightColor;
    COLOR _dayColorSunday;
    COLOR _dayColorSaturday;
    cBMP15 _dayNumbers;  // index 10 means colon
    cBMP15 _dayNumbersSecond;
    cBMP15 _yearNumbers;
    cBMP15 _dateSelection;
    bool _showYear;
    bool _showMonth;
    bool _showDayX;
    bool _showDay;
    bool _colonShow;
    int _fixOnes;
    u32 _lastDate;
};

typedef t_singleton<cCalendar> calendar_s;
inline cCalendar& calendar() {
    return calendar_s::instance();
}
