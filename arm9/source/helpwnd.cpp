/*
    helpwnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "cachedinifile.h"
#include "helpwnd.h"
#include "fontfactory.h"
#include "language.h"
#include "msgbox.h"
#include "uisettings.h"
#include "version.h"
#include "windowmanager.h"

void cHelpWnd::showModal(cWindow* parent) {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    u32 w = 200;
    u32 h = 160;
    w = ini.GetInt("help window", "w", w);
    h = ini.GetInt("help window", "h", h);
    cHelpWnd* modal = new cHelpWnd((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2, w, h, parent, LANG("help window", "title"));
    
    windowManager().addModal(modal);
}

cHelpWnd::cHelpWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
    : cForm(x, y, w, h, parent, text), _buttonOK(0, 0, 46, 18, this, "\x01 OK") {
    s16 buttonY = size().y - _buttonOK.size().y - 4;

    _buttonOK.setStyle(cButton::press);
    _buttonOK.setText("\x02 " + LANG("setting window", "cancel"));
    _buttonOK.setTextColor(uis().buttonTextColor);
    _buttonOK.loadAppearance(SFN_BUTTON3);
    _buttonOK.clicked.connect(this, &cHelpWnd::onOK);
    addChildWindow(&_buttonOK);

    s16 nextButtonX = size().x;

    s16 buttonPitch = _buttonOK.size().x + 8;
    buttonPitch = _buttonOK.size().x + 8;
    nextButtonX -= buttonPitch;
    _buttonOK.setRelativePosition(cPoint(nextButtonX, buttonY));

    loadAppearance("");

    for (size_t i = 0; i < 8; ++i) {
        std::string textIndex = formatString("item%d", i);
        _helpText += LANG("help window", textIndex);
        _helpText += "\n";
    }
    _helpText = formatString(_helpText.c_str(), 7, 1, 2, 4, 3, 5, 6, "START", "SELECT");

    std::string ndsbsVer = fsManager().resolveSystemPath("/_nds/release-bootstrap.ver");
    
    char ndsbsBuffer[256];
    _helpText += "https://github.com/maniman303\n";
    _helpText += formatString("AKMenu-Mako %s.%s ", AKMENU_VERSION_MAIN, AKMENU_VERSION_SUB);

    if(gs().pico){
        _helpText += formatString("\n%s %s ", AKMENU_PICO_NAME, AKMENU_LOADER_VERSION);
    }
    else if(access(ndsbsVer.c_str(), F_OK) == 0 && gs().pico == false){
        FILE* file = fopen(ndsbsVer.c_str(), "r");
        if (file) {
            if (fgets(ndsbsBuffer, sizeof(ndsbsBuffer), file)) {
                _helpText += formatString("\n%s %s ", AKMENU_LOADER_NAME, ndsbsBuffer);
            }
            fclose(file);
        }
    }
    else{
        _helpText += formatString("\n%s %s ", AKMENU_LOADER_NAME, AKMENU_LOADER_VERSION);
    }
}

cHelpWnd::~cHelpWnd() {}

void cHelpWnd::draw() {
    _renderDesc.draw(windowRectangle(), _engine);
    int titleOffset = _renderDesc.titleSize().y;

    gdi().setPenColor(uiSettings().formTextColor, _engine);
    gdi().textOutRect(position().x + 8, position().y + titleOffset + 2 + uiSettings().thickness, size().x - 8,
                      size().y - 20 - 2 * uiSettings().thickness, _helpText.c_str(), _engine);
    cForm::draw();
}

bool cHelpWnd::processKeyMessage(cKeyMessage message) {
    if (message.isKeyUp(KEY_A) || message.isKeyUp(KEY_B)) {
        onOK();
        return true;
    }

    return false;
}

cWindow& cHelpWnd::loadAppearance(const std::string& aFileName) {
    _renderDesc.loadData(SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);
    _renderDesc.setTitleText(_text);
    return *this;
}

void cHelpWnd::onOK() {
    cForm::onOK();
}

void cHelpWnd::onShow() {
    centerScreen();
}
