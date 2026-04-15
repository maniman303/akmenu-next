/*
    uisettings.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "uisettings.h"
#include "cachedinifile.h"
#include "../systemfilenames.h"

cUISettings::cUISettings() {
    showCalendar = true;
    formFrameColor = RGB15(23, 25, 4);
    formBodyColor = RGB15(30, 29, 22);
    formBodyColor2 = RGB15(30, 29, 22);
    formTextColor = RGB15(17, 12, 0);
    formTextHighLightColor = RGB15(17, 12, 0);
    formTitleTextColor = RGB15(11, 11, 11);
    buttonTextColor = RGB15(17, 12, 0);
    spinBoxNormalColor = RGB15(0, 0, 31);
    spinBoxFocusColor = RGB15(0, 31, 0);
    spinBoxTextColor = RGB15(31, 31, 31);
    spinBoxTextHighLightColor = RGB15(31, 31, 31);
    spinBoxFrameColor = RGB15(11, 11, 11);
    listViewBarColor1 = RGB15(0, 11, 19);
    listViewBarColor2 = RGB15(0, 5, 9);
    listTextColor = 0;
    listTextHighLightColor = 0;
    popMenuTextColor = RGB15(0, 0, 0);
    popMenuTextHighLightColor = RGB15(31, 31, 31);
    popMenuBarColor = RGB15(0, 11, 19);
    thickness = 1;
}

cUISettings::~cUISettings() {}

void cUISettings::loadSettings() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);

    showCalendar = ini.GetInt("global settings", "showCalendar", showCalendar);
    formFrameColor = ini.GetInt("global settings", "formFrameColor", formFrameColor) | BIT(15);
    formBodyColor = ini.GetInt("global settings", "formBodyColor", formBodyColor) | BIT(15);
    formBodyColor2 = ini.GetInt("global settings", "formBodyColor2", formBodyColor) | BIT(15);
    formTextColor = ini.GetInt("global settings", "formTextColor", formTextColor) | BIT(15);
    formTextHighLightColor = ini.GetInt("global settings", "formTextHighLightColor", formTextColor) | BIT(15);
    formTitleTextColor = ini.GetInt("global settings", "formTitleTextColor", formTitleTextColor) | BIT(15);
    buttonTextColor = ini.GetInt("global settings", "buttonTextColor", buttonTextColor) | BIT(15);
    spinBoxNormalColor = ini.GetInt("global settings", "spinBoxNormalColor", spinBoxNormalColor) | BIT(15);
    spinBoxFocusColor = ini.GetInt("global settings", "spinBoxFocusColor", spinBoxFocusColor) | BIT(15);
    spinBoxTextColor = ini.GetInt("global settings", "spinBoxTextColor", spinBoxTextColor) | BIT(15);
    spinBoxTextHighLightColor = ini.GetInt("global settings", "spinBoxTextHiLightColor", spinBoxTextHighLightColor) | BIT(15);
    spinBoxTextNamedColor = ini.GetInt("global settings", "spinBoxTextNamedColor", spinBoxTextColor) | BIT(15);
    spinBoxFrameColor = ini.GetInt("global settings", "spinBoxFrameColor", spinBoxFrameColor) | BIT(15);
    listViewBarColor1 = ini.GetInt("global settings", "listViewBarColor1", listViewBarColor1) | BIT(15);
    listViewBarColor2 = ini.GetInt("global settings", "listViewBarColor2", listViewBarColor2) | BIT(15);
    listTextColor = ini.GetInt("global settings", "listTextColor", listTextColor) | BIT(15);
    listTextHighLightColor =
            ini.GetInt("global settings", "listTextHighLightColor", listTextHighLightColor) | BIT(15);
    popMenuTextColor = ini.GetInt("global settings", "popMenuTextColor", popMenuTextColor) | BIT(15);
    popMenuTextHighLightColor =
            ini.GetInt("global settings", "popMenuTextHighLightColor", popMenuTextHighLightColor) | BIT(15);
    popMenuBarColor = ini.GetInt("global settings", "popMenuBarColor", popMenuBarColor) | BIT(15);
    thickness = ini.GetInt("global settings", "thickness", thickness);
}
