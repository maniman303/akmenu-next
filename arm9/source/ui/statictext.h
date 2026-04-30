/*
    statictext.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include "bmp15.h"
#include "window.h"

class cStaticText : public cWindow {
  public:
    cStaticText(cWindow* parent);
    cStaticText(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);

    ~cStaticText();

  public:
    void draw() override;
    void setTextColor(COLOR color);
    void setFont(bool primary);
    void setCentered(bool centered);
  protected:
    bool _centered;
    bool _primaryFont;
    COLOR _textColor;
};
