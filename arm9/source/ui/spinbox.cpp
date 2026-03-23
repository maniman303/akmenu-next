/*
    spinbox.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "spinbox.h"
#include "fontfactory.h"
#include "ui.h"
#include <math.h>

//#include "windowmanager.h"

namespace akui {

cSpinBox::cSpinBox(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text) : cSpinBox(x, y, w, h, false, parent, text) {}

cSpinBox::cSpinBox(s32 x, s32 y, u32 w, u32 h, bool namedAppearance, cWindow* parent, const std::string& text)
    : cForm(x, y, w, h, parent, text),
      _prevButton(0, 0, 0, 0, this, ""),
      _nextButton(0, 0, 0, 0, this, ""),
      _leftBg(this),
      _middleBg(this),
      _rightBg(this),
      _itemText(0, 0, 0, 0, this, "spinbox"),
      _namedAppearance(namedAppearance) {
    _normalColor = uiSettings().spinBoxNormalColor;  // RGB15( 0, 0, 31 );
    _focusedColor = uiSettings().spinBoxFocusColor;  // RGB15( 0, 31, 0 );
    _frameColor = uiSettings().spinBoxFrameColor;
    _itemText.setTextColor(uiSettings().spinBoxTextColor);
    _itemText.setFont(false);

    _prevButton.pressed.connect(this, &cSpinBox::selectPrev);
    _prevButton.pressed.connect(this, &cSpinBox::onCmponentClicked);
    _nextButton.pressed.connect(this, &cSpinBox::selectNext);
    _nextButton.pressed.connect(this, &cSpinBox::onCmponentClicked);

    addChildWindow(&_itemText);
    addChildWindow(&_prevButton);
    addChildWindow(&_nextButton);

    _itemText.setTextColor(RGB15(31, 31, 31));

    u8 cx = 0;
    _prevButton.setRelativePosition(cPoint(cx, 0));

    cx = 0 + _prevButton.windowRectangle().width();
    _itemText.setRelativePosition(cPoint(cx, 0));
    _itemText.setSize(cSize(w - 18 * 2, 18));

    cx = windowRectangle().width() - _nextButton.windowRectangle().width();
    _nextButton.setRelativePosition(cPoint(cx, 0));

    selectItem(0);
}

cSpinBox::~cSpinBox() {}

void cSpinBox::selectItem(u32 id) {
    // danger !!!! it may cause system halt when cSpinBox destruct
    // windowManager().setFocusedWindow( this );
    _selectedItemId = id;
    if (_selectedItemId >= _items.size()) return;

    _itemText.setText(_items[_selectedItemId]);

    // s32 textWidth = _items[_selectedItemId].length() * 6;
    // s32 textHeight = 12;
    //_itemText.setRelativePosition( cPoint((_size.x - textWidth) >> 1, (_size.y - textHeight) >> 1)
    //);

    arrangeButton();
    // TODO: Arrange header
    arrangeText();
    arrangeChildren();
    changed(this);
}

void cSpinBox::selectNext() {
    if (_items.size() - 1 == _selectedItemId) return;

    selectItem(_selectedItemId + 1);
}

void cSpinBox::selectPrev() {
    if (0 == _selectedItemId) return;

    selectItem(_selectedItemId - 1);
}

void cSpinBox::insertItem(const std::string& item, u32 position) {
    if (position > _items.size()) return;

    _items.insert(_items.begin() + position, item);

    if (_items.size() == 1) {
        selectItem(0);
    }
}

void cSpinBox::removeItem(u32 position) {
    if (position > _items.size() - 1) return;

    _items.erase(_items.begin() + position);
}

void cSpinBox::setTextColor(COLOR color) {}

void cSpinBox::draw() {
    // draw bar
    u16 barColor = _normalColor;
    if (_namedAppearance) {
        _itemText.setTextColor(uiSettings().spinBoxTextNamedColor);
    } else if (isActive()) {
        barColor = _focusedColor;
        _itemText.setTextColor(uiSettings().spinBoxTextHighLightColor);
    } else {
        _itemText.setTextColor(uiSettings().spinBoxTextColor);
    }

    bool drawBg = _leftBg.valid() || _middleBg.valid() || _rightBg.valid();
    if (drawBg) {
        s32 x = _prevButton.position().x + _prevButton.size().x;
        s32 y = _prevButton.position().y;
        _leftBg.draw(x, y);

        x += _leftBg.size().x;
        s32 middleWidth = _middleBg.size().x;
        if (middleWidth > 0) {
            s32 titleWidth = _nextButton.position().x - x - _rightBg.size().x;
            s32 repeats = (titleWidth / middleWidth) + 1;
            for (s32 i = 0; i < repeats; i++) {
                _middleBg.draw(x + (i * middleWidth), y);
            }
        }

        x = _nextButton.position().x - _rightBg.size().x;
        _rightBg.draw(x, y);
    } else {
        gdi().setPenColor(barColor, _engine);

        u8 bodyX1 = _prevButton.position().x + _prevButton.size().x;
        u8 fillWidth = windowRectangle().size().x - _nextButton.size().x - _prevButton.size().x;
        gdi().fillRect(barColor, barColor, bodyX1, _position.y, fillWidth, _prevButton.size().y,
                    selectedEngine());

        gdi().setPenColor(_frameColor, _engine);
        gdi().frameRect(bodyX1, _position.y, fillWidth, _prevButton.size().y, uiSettings().thickness,
                        selectedEngine());
    }

    // draw previous button
    _prevButton.draw();

    // draw text
    _itemText.draw();

    // draw next button
    _nextButton.draw();
}

cWindow& cSpinBox::loadAppearance(const std::string& aFileName) {
    if (_namedAppearance) {
        _prevButton.loadAppearance(SFN_SPINBUTTON_L_NAMED);
        _nextButton.loadAppearance(SFN_SPINBUTTON_R_NAMED);

        _leftBg.loadAppearance(SFN_SPINBG_L_NAMED);
        _middleBg.loadAppearance(SFN_SPINBG_M_NAMED);
        _rightBg.loadAppearance(SFN_SPINBG_R_NAMED);
    }

    if (!_prevButton.valid()) {
        _prevButton.loadAppearance(SFN_SPINBUTTON_L);
    }

    if (!_nextButton.valid()) {
        _nextButton.loadAppearance(SFN_SPINBUTTON_R);
    }

    setSize(cSize(size().x, std::max(_prevButton.size().y, _nextButton.size().y)));

    return *this;
}

void cSpinBox::onGainedFocus() {}

void cSpinBox::onResize() {
    dbg_printf("spin box on resize\n");
    arrangeButton();
    // TODO: Arrange header
    arrangeText();
    arrangeChildren();
}

void cSpinBox::onMove() {
    arrangeButton();
    // TODO: Arrange header
    arrangeText();
    arrangeChildren();
}

void cSpinBox::arrangeText() {
    s32 textWidth = _items.size() ? font().TextWidth(_items[_selectedItemId]) : 0;
    if (textWidth > _itemText.size().x) textWidth = _itemText.size().x;

    if (_namedAppearance) {
        int xOffset = _leftBg.size().x == 0 ? 5 : _leftBg.size().x;
        int positionX = _prevButton.size().x + xOffset;
        _itemText.setRelativePosition(cPoint(positionX, (_size.y - font().GetHeight()) >> 1));
        _itemText.setFont(true);
    } else {
        _itemText.setRelativePosition(cPoint((_size.x - textWidth) >> 1, (_size.y - fontSecondary().GetHeight()) >> 1));
        _itemText.setFont(false);
    }
}

void cSpinBox::arrangeButton() {
    u8 x = 0;
    _prevButton.setRelativePosition(cPoint(x, (_size.y - _prevButton.size().y) / 2));

    x = _prevButton.size().x;
    u8 fontHeight = _namedAppearance ? font().GetHeight() : fontSecondary().GetHeight();
    _itemText.setRelativePosition(cPoint(x, (_size.y - fontHeight) / 2));

    x = size().x - _nextButton.size().x;
    _nextButton.setRelativePosition(cPoint(x, (_size.y - _nextButton.size().y)));
}

void cSpinBox::onCmponentClicked() {
    componentClicked(this);
}

}  // namespace akui
