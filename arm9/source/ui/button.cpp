/*
    button.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "button.h"
#include "fontfactory.h"
#include "uisettings.h"
#include "window.h"

cButton::cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text) : cButton(x, y, w, h, parent, text, true) {}

cButton::cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text, bool hasAlpha)
    : cWindow(parent, text) {
    _captured = false;
    _state = up;
    setSize(cSize(w, h));
    setRelativePosition(cPoint(x, y));
    _textColor = uiSettings().buttonTextColor;  // RGB15(0,0,0) | BIT(15);
    _style = single;
    _alignment = center;
    _isFocusable = false;
    _hasAlpha = hasAlpha;
}

cButton::~cButton() { }

void cButton::draw() {
    if (!_background.valid()) {
        return;
    }

    const cPoint topLeft = position();
    const cPoint bottomRight = position() + size();
    cRect area(topLeft, bottomRight);
    
    const u32* pBuffer = NULL;
    u32 height = 0;
    if (_background.valid()) {
        pBuffer = _background.buffer();
        height = _background.height();

        if (style() != cButton::single) {
            height /= 2;

            if (state() == cButton::down) {
                pBuffer += (_background.pitch() * _background.height() / 8);
            }
        }
    }

    if (pBuffer != NULL) {
        if (hasAlpha()) {
            gdi().maskBlt(pBuffer, area.position().x, area.position().y, _background.width(), height, selectedEngine());
        } else {
            gdi().bitBlt(pBuffer, area.position().x, area.position().y, _background.width(), height, selectedEngine());
        }
    }

    std::string text = _text;
    if (_text.empty()) {
        return;
    }

    u32 textPixels = font().TextWidth(_text);
    u32 textX = 0, textY = area.position().y + ((area.size().y - SYSTEM_FONT_HEIGHT) >> 1) + 1;
    switch (alignment()) {
        case cButton::center:
            textX = area.position().x + ((area.size().x - textPixels) >> 1);
            break;
        case cButton::right:
            textX = area.position().x + (area.size().x - textPixels - 4);
            break;
        default:
            textX = area.position().x + 4;
            break;
    }

    if (cButton::down == state()) {
        textX++;
        textY++;
    }

    gdi().setPenColor(textColor(), selectedEngine());
    gdi().textOutRect(textX, textY, area.size().x, area.size().y, _text.c_str(), selectedEngine());
}

cWindow& cButton::loadAppearance(const std::string& aFileName) {
    _background = createBMP15FromFile(aFileName);
    if (_background.valid()) {
        int height = size().y;
        if (style() == cButton::single) {
            height = _background.height();
        } else {
            height = _background.height() / 2;
        }
            
        setSize(cSize(_background.width(), height));
    }

    return *this;
}

bool cButton::valid() const {
    return _background.valid();
}

bool cButton::isTouchFocusable() {
    return false;
}

void cButton::setIsFocusable(bool isFocusable) {
    _isFocusable = isFocusable;
}

bool cButton::processKeyMessage(cKeyMessage message) {
    if (!isVisible() || !isFocused()) {
        return false;
    }

    if (message.isKeyUp(KEY_A)) {
        onClicked();
        clicked();
        return true;
    }

    return false;
}

bool cButton::processTouchMessage(cTouchMessage message) {
    if (!isVisible()) {
        return false;
    }

    cRect myRect(position().x, position().y, position().x + size().x, position().y + size().y);
    if (message.up()) {
        if (!_captured) {
            return false;
        }

        _captured = false;
        _state = up;

        if (myRect.surrounds(message.position())) {
            onClicked();
            clicked();
        } else {
            onReleased();
        }

        return true;
    }

    if (message.down() && myRect.surrounds(message.position())) {
        onPressed();
        pressed();
        _captured = true;
        _state = down;
        
        return true;
    }

    return false;
}

void cButton::onPressed() {}

void cButton::onReleased() {}

void cButton::onClicked() {}
