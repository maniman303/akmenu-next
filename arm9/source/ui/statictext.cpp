/*
    statictext.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "statictext.h"
#include "fontfactory.h"
#include "uisettings.h"
//#include "gdi.h"

namespace akui {

cStaticText::cStaticText(cWindow* parent) : cStaticText(0, 0, 0, 0, parent, "") {}

cStaticText::cStaticText(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
    : cWindow(parent, text) {
    _size.x = w;
    _size.y = h;
    _primaryFont = true;
    _centered = false;
    _textColor = uiSettings().formTextColor;  //(RGB15(31,31,31))
    _isFocusable = false;

    setRelativePosition(cPoint(x, y));
}

cStaticText::~cStaticText() {}

void cStaticText::draw() {
    if (_text.empty()) {
        return;
    }

    gdi().setPenColor(_textColor, _engine);
    gdi().textOutRect(position().x, position().y, size().x, size().y, _text.c_str(), selectedEngine(), _primaryFont ? font() : fontSecondary());
}

cWindow& cStaticText::loadAppearance(const std::string& aFileName) {
    return *this;
}

void cStaticText::setTextColor(COLOR color) {
    _textColor = color;
}

void cStaticText::setFont(bool primary) {
    _primaryFont = primary;
}

void cStaticText::setCentered(bool centered) {
    _centered = centered;
}

}  // namespace akui
