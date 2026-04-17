/*
    datetime.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "datetime.h"
#include <string.h>  //memset

#define IS_LEAP(n) ((!(((n) + 1900) % 400) || (!(((n) + 1900) % 4) && (((n) + 1900) % 100))) != 0)

const char* cDateTime::weekdayStrings[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

cDateTime::cDateTime() {
    set = false;
}

void cDateTime::FillTimeParts(void) {
    if (set) {
        return;
    }

    time_t epochTime;
    if (time(&epochTime) == (time_t)-1) {
        memset(&iTimeParts, 0, sizeof(iTimeParts));
    } else {
        localtime_r(&epochTime, &iTimeParts);
    }

    set = true;
}

u16 cDateTime::year() {
    FillTimeParts();
    return iTimeParts.tm_year + 1900;
}

u8 cDateTime::month() {
    FillTimeParts();
    return iTimeParts.tm_mon + 1;
}

u8 cDateTime::day() {
    FillTimeParts();
    return iTimeParts.tm_mday;
}

u8 cDateTime::weekday() {
    FillTimeParts();
    return iTimeParts.tm_wday;
}

u32 cDateTime::now() {
    u32 res = ((u32)year()) << 16;
    res = res | ((u16)month()) << 8;
    res = res | day();
    return res;
}

u8 cDateTime::hours() {
    FillTimeParts();
    return iTimeParts.tm_hour;
}

u8 cDateTime::minutes() {
    FillTimeParts();
    return iTimeParts.tm_min;
}

u8 cDateTime::seconds() {
    FillTimeParts();
    return iTimeParts.tm_sec;
}

u8 cDateTime::daysOfMonth() {
    return (28 | (((IS_LEAP(year()) ? 62648028 : 62648012) >> (month() * 2)) & 3));
}

u8 cDateTime::weekDayOfMonthFirstDay() {
    return (weekday() + 7 - ((day() - 1) % 7)) % 7;
}

u64 cDateTime::secondsInDay(void) {
    FillTimeParts();
    u8 hours = iTimeParts.tm_hour;
    u16 minutes = hours * 60 + iTimeParts.tm_min;
    u64 seconds = minutes * 60 + iTimeParts.tm_sec;

    return seconds;
}

std::string cDateTime::getDateString() {
    // FillTimeParts();
    return formatString("%d/%d%/%d %s\n", year(), month(), day(), weekdayStrings[weekday()]);
}

std::string cDateTime::getTimeString() {
    // FillTimeParts();
    return formatString("%d:%d%:%d\n", hours(), minutes(), seconds());
}

std::string cDateTime::getTimeStampString() {
    // FillTimeParts();
    return formatString("%04d%02d%02d%02d%02d%02d", year(), month(), day(), hours(), minutes(),
                        seconds());
}

void cDateTime::purge() {
    set = false;
}
