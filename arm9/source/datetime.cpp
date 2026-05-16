/*
    datetime.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include <string.h>
#include "datetime.h"
#include "blockds/ndstypes.h"

inline bool isLeapYear(u16 year) {
    return (year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0));
}

const char* cDateTime::weekdayStrings[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

cDateTime::cDateTime() {
    fillTimeParts();
}

ARM_CODE LIBNDS_NOINLINE
void ITCM_FUNC(cDateTime::fillTimeParts)(void) {
    time_t epochTime;
    if (time(&epochTime) == (time_t)-1) {
        memset(&iTimeParts, 0, sizeof(iTimeParts));
    } else {
        localtime_r(&epochTime, &iTimeParts);
    }
}

ARM_CODE LIBNDS_NOINLINE
u16 ITCM_FUNC(cDateTime::year)() {
    return iTimeParts.tm_year + 1900;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::month)() {
    return iTimeParts.tm_mon + 1;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::day)() {
    return iTimeParts.tm_mday;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::weekday)() {
    return iTimeParts.tm_wday;
}

ARM_CODE LIBNDS_NOINLINE
u32 ITCM_FUNC(cDateTime::now)() {
    u32 res = ((u32)year()) << 16;
    res = res | ((u16)month()) << 8;
    res = res | day();
    return res;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::hours)() {
    return iTimeParts.tm_hour;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::minutes)() {
    return iTimeParts.tm_min;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::seconds)() {
    return iTimeParts.tm_sec;
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::daysOfMonth)() {
    return (28 | (((isLeapYear(year()) ? 62648028 : 62648012) >> (month() * 2)) & 3));
}

ARM_CODE LIBNDS_NOINLINE
u8 ITCM_FUNC(cDateTime::weekDayOfMonthFirstDay)() {
    return (weekday() + 7 - ((day() - 1) % 7)) % 7;
}

ARM_CODE LIBNDS_NOINLINE
u64 ITCM_FUNC(cDateTime::secondsInDay)(void) {
    u8 hours = iTimeParts.tm_hour;
    u16 minutes = hours * 60 + iTimeParts.tm_min;
    u64 seconds = minutes * 60 + iTimeParts.tm_sec;

    return seconds;
}

std::string cDateTime::getDateString() {
    return formatString("%d/%d%/%d %s\n", year(), month(), day(), weekdayStrings[weekday()]);
}

std::string cDateTime::getTimeString() {
    return formatString("%d:%d%:%d\n", hours(), minutes(), seconds());
}

std::string cDateTime::getTimeStampString() {
    return formatString("%04d%02d%02d%02d%02d%02d", year(), month(), day(), hours(), minutes(), seconds());
}
