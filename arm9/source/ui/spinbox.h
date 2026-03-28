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
    struct cSpinItem {
      std::string _text;
      u32 _position;
      cSpinItem(std::string text, u32 position) : _text(text), _position(position) {}
    };

  public:
    void draw();

    cWindow& loadAppearance(const std::string& aFileName);

    u32 selectedItemId() { return _selectedItemId; }

    void selectItem(u32 id);

    void selectNext();

    void selectPrev();

    void insertItem(const std::string& item, u32 position);

    void insertItem(cSpinItem item);

    void removeItem(u32 position);

    void setTextColor(COLOR color);

    void onCmponentClicked();
    Signal1<cSpinBox*> componentClicked;
    Signal1<cSpinBox*> changed;

  protected:
    void onResize();

    void onMove();

    void onGainedFocus();

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
    cStaticText _itemText;
    bool _namedAppearance;
    u32 _selectedItemId;
    std::vector<cSpinItem> _items;
};

}  // namespace akui
