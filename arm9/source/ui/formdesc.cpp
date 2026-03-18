/*
    formdesc.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "formdesc.h"
#include "fontfactory.h"
#include "ui.h"

//#include "globalsettings.h"

namespace akui {

// 边框颜色：b5c71f
//    23, 25, 4
// 框内背景色：eeebae
// 30, 29, 22

cFormDesc::cFormDesc() {
    _bodyColor = uiSettings().formBodyColor;    // RGB15(30,29,22);
    _frameColor = uiSettings().formFrameColor;  // RGB15(23,25,4);
    _centerTitle = false;
}

cFormDesc::~cFormDesc() {}

void cFormDesc::draw(const cRect& area, GRAPHICS_ENGINE engine) const {
    if (_topleft.valid()) {
        gdi().maskBlt(_topleft.buffer(), area.position().x, area.position().y, _topleft.width(),
                      _topleft.height(), engine);
    }

    if (_middle.valid()) {
        for (u32 i = 0; i < _middle.height(); ++i) {
            COLOR lineColor = _middle.buffer()[i] & 0xFFFF;
            gdi().setPenColor(lineColor, engine);
            gdi().fillRect(lineColor, lineColor, area.position().x + _topleft.width(),
                           area.position().y + i,
                           area.size().x - _topleft.width() - _topright.width(), 1, engine);
        }
    }

    if (_topright.valid()) {
        gdi().maskBlt(_topright.buffer(), area.position().x + area.size().x - _topright.width(),
                      area.position().y, _topright.width(), _topright.height(), engine);
    }

    if (_titleText != "") {
        gdi().setPenColor(uiSettings().formTitleTextColor, engine);

        u32 textX = area.position().x + 8;
        u32 textY = area.position().y + (((_topleft.height() - gs().fontHeight)) >> 1) + 1;
        if (_centerTitle) {
            u32 titleLength = font().TextLenght(_titleText);
            textX = (area.size().x - titleLength) / 2;
        }

        gdi().textOut(textX, textY, _titleText.c_str(), engine);
    }

    gdi().setPenColor(_bodyColor, engine);
    gdi().fillRect(_bodyColor, _bodyColor, area.topLeft().x, area.topLeft().y + _topleft.height(),
                   area.width(), area.height() - _topleft.height(), engine);

    gdi().setPenColor(_frameColor, engine);
    gdi().frameRect(area.topLeft().x, area.topLeft().y + _topleft.height(), area.width(),
                    area.height() - _topleft.height(), uiSettings().thickness, engine);
}

void cFormDesc::loadData(const std::string& topleftBmpFile, const std::string& toprightBmpFile,
                         const std::string& middleBmpFile) {
    _topleft = createBMP15FromFile(topleftBmpFile);
    _topright = createBMP15FromFile(toprightBmpFile);
    _middle = createBMP15FromFile(middleBmpFile);
}

cSize cFormDesc::size() {
    if (_topleft.valid()) {
        return cSize(_topleft.width(), _topleft.height());
    }

    return cSize(0, 0);
}

void cFormDesc::setTitleText(const std::string& text) {
    setTitleText(text, false);
}

void cFormDesc::setTitleText(const std::string& text, bool centerTitle) {
    _titleText = text;
    _centerTitle = centerTitle;
}

}  // namespace akui
