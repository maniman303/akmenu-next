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

    fillLines();

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
    
    for (size_t i = 0; i < _lines.size(); i++) {
        u32 textHeight = textFont.GetHeight() + textFont.GetDescend();
        s16 posX = position().x + (size().x - _lines[i].width) / 2;
        s16 posY = position().y + i * textHeight;
        gdi().textOutRect(posX, posY, size().x, size().y, _lines[i].text.c_str(), selectedEngine(), textFont);
    }
}

void cStaticText::setTextColor(COLOR color) {
    _textColor = color;
}

void cStaticText::setFont(bool primary) {
    _primaryFont = primary;
    fillLines();
}

void cStaticText::setCentered(bool centered) {
    _centered = centered;
    fillLines();
}

void cStaticText::onTextChanged() {
    fillLines();
}

void cStaticText::fillLines() {
    _lines.clear();
    
    if (!_centered) {
        return;
    }

    cFont& textFont = _primaryFont ? font() : fontSecondary();
    std::string content = textFont.BreakLine(_text, size().x);
    std::vector<std::string> lines = splitLines(content);

    for (const std::string& line : lines) {
        _lines.emplace_back(line, textFont.TextWidth(line));
    }
}
