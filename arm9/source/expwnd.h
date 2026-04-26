/*
    expwnd.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>
#include "dsrom.h"
#include "form.h"
#include "formdesc.h"
#include "spinbox.h"
#include "statictext.h"

class cExpWnd : public akui::cForm {
  public:
    cExpWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);
    ~cExpWnd();

  public:
    static void showModal(cWindow* parent);
    void draw();
    bool processKeyMessage(cKeyMessage message) override;
    cWindow& loadAppearance(const std::string& aFileName);

  protected:
    void onOK() override;
    void onCancel() override;
    void onRAM();
    void onSRAM();
    void onShow() override;
    cButton _buttonOK;
    cButton _buttonCancel;
    cButton _buttonRAM;
    cButton _buttonSRAM;
    akui::cSpinBox _Rumble;
    cStaticText _Label;
    akui::cFormDesc _renderDesc;
    std::string _romName;
};
