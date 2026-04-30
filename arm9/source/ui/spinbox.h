/*
    spinbox.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>
#include <vector>
#include "button.h"
#include "form.h"
#include "statictext.h"
#include "image.h"

namespace akui {

class cSpinBox : public cForm {
  public:
    cSpinBox(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text);
    cSpinBox(s32 x, s32 y, u32 w, u32 h, bool namedAppearance, cWindow* parent, const std::string& text);
    ~cSpinBox();

  public:
    void draw();

    u32 selectedItemId() { return _selectedItemId; }

    void selectItem(u32 id);

    void selectNext();

    void selectPrev();

    void insertItem(const std::string& item, u32 position);

    void removeItem(u32 position);

    void setPrefixIcon(const std::string& filename);

    Signal1<cSpinBox*> changed;

  protected:
    void loadAppearance();

    void onResize();

    void onMove();

    void onFocused();

    void arrangeButton();

    void arrangeText();

  protected:
    COLOR _focusedColor;
    COLOR _normalColor;
    COLOR _frameColor;
    cButton _prevButton;
    cButton _nextButton;
    cImage _leftBg;
    cImage _middleBg;
    cImage _rightBg;
    cImage _prefixIcon;
    cStaticText _itemText;
    bool _namedAppearance;
    u32 _selectedItemId;
    std::vector<std::string> _items;
};

}  // namespace akui
