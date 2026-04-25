/*
    mainwnd.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "button.h"
#include "form.h"
#include "keymessage.h"
#include "mainlist.h"
#include "settingwnd.h"
#include "spinbox.h"
#include "startmenu.h"
#include "touchmessage.h"
#include "rominfownd.h"
#include "ui/focusborder.h"

class cMainWnd : public akui::cForm {
  public:
    cMainWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);

    ~cMainWnd();

  public:
    bool processKeyMessage(cKeyMessage message) override;
    bool processTouchMessage(cTouchMessage message) override;
    cWindow& loadAppearance(const std::string& aFileName) override;
    void update() override;
    void init();
    void draw();

    cMainList* _mainList;

  protected:
    void onGainedFocus() override;
    void onMainListSelItemClicked(u32 index);
    void setFocusedChild(cWindow* child) override;
    void startMenuItemClicked(s16 i);
    void startButtonClicked();
    void brightnessButtonClicked();
    void showSettings(void);
    void saveSettings(cSettingWnd* settingWnd);
    void onFolderChanged();
    void showFileInfo();
    void saveFileInfo(cRomInfoWnd* romInfoWnd);
    void launchSelected();

    cStartMenu* _startMenu;
    akui::cButton* _startButton;
    akui::cButton* _brightnessButton;
    akui::cButton* _folderUpButton;
    akui::cStaticText* _folderText;
    cFocusBorder* _focusBorder;
};
