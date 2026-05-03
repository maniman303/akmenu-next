/*
    startmenu.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "startmenu.h"
#include "cachedinifile.h"
#include "favorites.h"
#include "language.h"
#include "mainlist.h"
#include "systemfilenames.h"
#include "windowmanager.h"

void cStartMenu::init() {
    addItem(START_MENU_ITEM_FAVORITES, LANG("start menu", "Set favorite"));
    addItem(START_MENU_ITEM_INFO, LANG("start menu", "Info"));
    addItem(START_MENU_ITEM_SETTING, LANG("start menu", "Setting"));
    addItem(START_MENU_ITEM_HELP, LANG("start menu", "Help"));
    //addItem(START_MENU_ITEM_TOOLS, LANG("start menu", "Tools"));
    loadAppearance(SFN_STARTMENU_BG);
    dbg_printf("startmenu ok\n");
}

bool cStartMenu::processKeyMessage(cKeyMessage message) {
    if (message.isKeyUp(KEY_START)) {
        close(false);
        return true;
    }

    return cPopMenu::processKeyMessage(message);
}

void cStartMenu::loadAppearance(const std::string& aFileName) {
    cPopMenu::loadAppearance(aFileName);
    setSize(cSize(_background.width(), _background.height()));

    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    int ix = ini.GetInt("start menu", "itemX", 4);
    int iy = ini.GetInt("start menu", "itemY", 12);
    int x = ini.GetInt("start menu", "x", 4);
    int y = ini.GetInt("start menu", "y", 4);
    setRelativePosition(cPoint(x, y));
    _itemTopLeftPoint = cPoint(ix, iy);
    _itemHeight = ini.GetInt("start menu", "itemHeight", 16);
    _itemWidth = ini.GetInt("start menu", "itemWidth", 0);
    _barLeft = ini.GetInt("start menu", "barLeft", 2);
    if (_itemWidth == 0 && _barLeft * 2 > _size.x) _barLeft = 0;
}

cWindow& cStartMenu::showForFile(const std::string& fileName) {
    clearItem();

    if (fileName.empty()) {
        addItem(START_MENU_ITEM_FAVORITES, "");
    } else if (cFavorites::IsInFavorites(fileName)) {
        addItem(START_MENU_ITEM_FAVORITES, LANG("start menu", "Unset favorite"));
    } else {
        addItem(START_MENU_ITEM_FAVORITES, LANG("start menu", "Set favorite"));
    }

    addItem(START_MENU_ITEM_INFO, LANG("start menu", "Info"));
    addItem(START_MENU_ITEM_SETTING, LANG("start menu", "Setting"));
    addItem(START_MENU_ITEM_HELP, LANG("start menu", "Help"));

    return show();
}
