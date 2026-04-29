/*
    button.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include "bmp15.h"
#include "rectangle.h"
#include "renderdesc.h"
#include "window.h"

class cButton : public cWindow {
  public:
    enum Style { single = 0, press = 1, toggle = 2 };
    enum Alignment { left, center, right };

    cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);
    cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text, bool hasAlpha);

    ~cButton();

  public:
    void draw() override;
    cWindow& loadAppearance(const std::string& aFileName) override;
    bool valid() const;
    bool isTouchFocusable() override;
    void setIsFocusable(bool isFocusable);
    bool processKeyMessage(cKeyMessage message) override;
    bool processTouchMessage(cTouchMessage message) override;
    void setTextColor(COLOR color) { _textColor = color; }
    COLOR textColor() { return _textColor; }
    void setStyle(Style style) { _style = style; }
    Style style() { return _style; }
    void setAlignment(Alignment alignment) { _alignment = alignment; }
    void setPressed(bool isPressed);
    Alignment alignment() { return _alignment; }
    bool hasAlpha() { return _hasAlpha; }

    Signal0 clicked;

  protected:
    bool _captured;
    u16 _state;
    COLOR _textColor;
    cBMP15 _background;
    Style _style;
    Alignment _alignment;
    bool _hasAlpha;
};
