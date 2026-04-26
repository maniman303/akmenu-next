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
#include "../stringtool.h"

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

    cFont& textFont = _primaryFont ? font() : fontSecondary();
    if (!_centered) {
        gdi().textOutRect(position().x, position().y, size().x, size().y, _text.c_str(), selectedEngine(), textFont);
        return;
    }
    
    std::string content = textFont.BreakLine(text(), size().x);
    std::vector<std::string> lines = splitLines(content);
    for (size_t i = 0; i < lines.size(); i++) {
        u32 textWidth = textFont.TextWidth(lines[i]);
        u32 textHeight = textFont.GetHeight() + textFont.GetDescend();
        s16 posX = position().x + (size().x - textWidth) / 2;
        s16 posY = position().y + i * textHeight;
        gdi().textOutRect(posX, posY, size().x, size().y, lines[i].c_str(), selectedEngine(), textFont);
    }
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
