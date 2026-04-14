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

namespace akui {

cButton::cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text) : cButton(x, y, w, h, parent, text, true) {}

cButton::cButton(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text, bool hasAlpha)
    : cWindow(parent, text), _renderDesc(NULL) {
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

cButton::~cButton() {
    if (_renderDesc) delete _renderDesc;
}

void cButton::draw() {
    if (NULL == _renderDesc) _renderDesc = new cButtonDesc();
    const cPoint topLeft = position();
    const cPoint bottomRight = position() + size();

    cRect rect(topLeft, bottomRight);
    _renderDesc->draw(rect, selectedEngine());
}

cWindow& cButton::loadAppearance(const std::string& aFileName) {
    _renderDesc = new cButtonDesc();
    _renderDesc->setButton(this);
    _renderDesc->loadData(aFileName);

    return *this;
}

bool cButton::valid() const {
    if (_renderDesc == NULL) {
        return false;
    }

    return _renderDesc->valid();
}

bool cButton::isTouchFocusable() {
    return false;
}

void cButton::setIsFocusable(bool isFocusable) {
    _isFocusable = isFocusable;
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

        if (myRect.surrounds(message.position())) {
            onClicked();
            clicked();
        } else {
            onReleased();
        }
        _captured = false;
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

///////////////////////////////// desc ////////////////
cButtonDesc::cButtonDesc() {
    _button = NULL;
    _textColor = RGB15(31, 31, 31);
}

cButtonDesc::~cButtonDesc() {
    // if( NULL != _background )
    //     destroyBMP15( _background );
}

void cButtonDesc::draw(const cRect& area, GRAPHICS_ENGINE engine) const {
    if (_button == NULL) {
        return;
    }

    const u32* pBuffer = NULL;
    u32 height = 0;
    if (_background.valid()) {
        pBuffer = _background.buffer();
        height = _background.height();
        if (_button->style() != cButton::single) {
            height /= 2;
            if (_button->state() == cButton::down) {
                pBuffer += (_background.pitch() * _background.height() / 8);
            }
        }
    }

    if (pBuffer != NULL) {
        if (_button->hasAlpha()) {
            gdi().maskBlt(pBuffer, area.position().x, area.position().y, _background.width(), height, _button->selectedEngine());
        } else {
            gdi().bitBlt(pBuffer, area.position().x, area.position().y, _background.width(), height, _button->selectedEngine());
        }
    }

    if (_button->text().empty()) {
        return;
    }

    u32 textPixels = font().TextWidth(_button->text());
    u32 textX = 0, textY = area.position().y + ((area.size().y - SYSTEM_FONT_HEIGHT) >> 1) + 1;
    switch (_button->alignment()) {
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

    if (cButton::down == _button->state()) {
        textX++;
        textY++;
    }
    gdi().setPenColor(_button->textColor(), _button->selectedEngine());
    gdi().textOutRect(textX, textY, area.size().x, area.size().y, _button->text().c_str(),
                      _button->selectedEngine());
}

void cButtonDesc::loadData(const std::string& filename) {
    int height = 16;
    if (_button) {
        height = _button->size().y;
    }

    _background = createBMP15FromFile(filename);
    if (_background.valid()) {
        if (_button->style() == cButton::single) {
            height = _background.height();
        } else {
            height = _background.height() / 2;
        }
            
        _button->setSize(cSize(_background.width(), height));
    }
}

bool cButtonDesc::valid() const {
    return _background.valid();
}

}  // namespace akui
