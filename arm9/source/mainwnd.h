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
    void draw() override;

    cMainList* _mainList;

  protected:
    void onFocused() override;
    void onMainListSelectionChanged(u32 index);
    void onMainListSelItemClicked(u32 index);
    void setFocusedChild(cWindow* child) override;
    void startMenuItemClicked(s16 i);
    void startMenuItemSelected(s16 i);
    void startMenuClosed();
    void startButtonClicked();
    void brightnessButtonClicked();
    void clockButtonClicked();
    void showSettings(void);
    void saveSettings(cSettingWnd* settingWnd);
    void onFolderChanged();
    void showFileInfo(u32 id);
    void saveFileInfo(cRomInfoWnd* romInfoWnd);
    void launchSelected();

    cStartMenu* _startMenu;
    cButton* _startButton;
    cButton* _brightnessButton;
    cButton* _clockButton;
    cButton* _folderUpButton;
    cStaticText* _folderText;
    cFocusBorder* _focusBorder;
};
