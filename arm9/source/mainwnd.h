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
    void update() override;
    void init();
    void draw() override;
    void onDisplayed() override;
    void onFocused() override;

    cMainList* _mainList;

  protected:
    void onMainListSelectionChanged(u32 index);
    void onMainListSelItemClicked(u32 index);
    void onMainListDirectoryChanged();
    void onMainListDirectoryReturned();
    void setFocusedChild(cWindow* child) override;
    void startMenuItemClicked(s16 i);
    void startMenuItemSelected(s16 i);
    void startMenuClosed(bool selected);
    void startButtonClicked();
    void brightnessButtonClicked();
    void fileInfoButtonClicked();
    void folderUpButtonClicked();
    void showSettings(void);
    void saveSettings(cSettingWnd* settingWnd);
    void showFileInfo(u32 id);
    void saveFileInfo(cRomInfoWnd* romInfoWnd);
    void launchSelected();

    bool _scheduleListFocus;
    cStartMenu* _startMenu;
    cButton* _startButton;
    cButton* _brightnessButton;
    cButton* _fileInfoButton;
    cButton* _folderUpButton;
    cStaticText* _folderText;
    cFocusBorder* _focusBorder;
};
