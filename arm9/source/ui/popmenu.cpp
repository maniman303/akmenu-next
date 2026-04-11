/*
    popmenu.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

//�

#include "popmenu.h"
#include "ui.h"
#include "fontfactory.h"
#include "timer.h"
#include "logger.h"
//#include "windowmanager.h"

namespace akui {

cPopMenu::cPopMenu(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
    : cWindow(parent, text) {
    setSize(cSize(w, h));
    setRelativePosition(cPoint(x, y));

    _selectedItemIndex = 0;
    _itemHeight = 0;
    _itemWidth = 0;
    _barLeft = 2;

    _textColor = uiSettings().popMenuTextColor;
    _textHighLightColor = uiSettings().popMenuTextHighLightColor;
    _barColor = uiSettings().popMenuBarColor;

    _renderDesc = new cBitmapDesc();
    _renderDesc->setBltMode(BM_MASKBLT);

    _skipTouch = false;
    _scrollTick = 0;
}

cPopMenu::~cPopMenu() {
    if (NULL != _renderDesc) delete _renderDesc;
}

void cPopMenu::popup() {
    show();
    return;
}

void cPopMenu::addItem(size_t index, const std::string& itemText) {
    if (index > _items.size()) index = _items.size();
    _items.insert(_items.begin() + index, itemText);
}

void cPopMenu::removeItem(size_t index) {
    if (index > _items.size() - 1) index = _items.size() - 1;
    _items.erase(_items.begin() + index);
}

size_t cPopMenu::itemCount() {
    return _items.size();
}

void cPopMenu::clearItem() {
    _items.clear();
}

void cPopMenu::draw() {
    _renderDesc->draw(windowRectangle(), selectedEngine());
    drawItems();
}

void cPopMenu::drawItems() {
    // 循环绘制item文字，遇见 selected 文字就先绘制选择条
    for (size_t i = 0; i < _items.size(); ++i) {
        s16 itemX = position().x + _itemTopLeftPoint.x;
        s16 itemY = position().y + i * _itemHeight + _itemTopLeftPoint.y;
        if (_selectedItemIndex == (s16)i) {
            s16 barX = position().x + _barLeft;
            s16 barY = itemY - 2;
            gdi().setPenColor(_barColor, _engine);
            gdi().fillRect(_barColor, _barColor, barX, barY, barWidth(), _itemHeight, _engine);
            gdi().setPenColor(_textHighLightColor, _engine);
        } else {
            gdi().setPenColor(_textColor, _engine);
        }
        gdi().textOut(itemX, itemY, _items[i].c_str(), _engine, fontSecondary());
    }
}

s16 cPopMenu::barWidth(void) {
    return _itemWidth ? _itemWidth : (_size.x - 2 * _barLeft);
}

bool cPopMenu::processKeyMessage(cKeyMessage message) {
    if (message.isKeyUp(KEY_A)) {
        hide();
        // logger().info("Selecting pop menu option.");
        itemClicked(_selectedItemIndex);
        return true;
    }

    if (message.isKeyUp(KEY_B)) {
        hide();
        return true;
    }

    if (message.isKeyDown(KEY_DOWN) || message.isKeyDown(KEY_UP)) {
        _scrollTick = timer().getTick();
        // logger().info("Scroll setup: " + std::to_string(_scrollTick));
    }

    u32 tickDiff = timer().getTick() - _scrollTick;
    if (message.isKeyDown(KEY_DOWN) || (message.isKeyHeld(KEY_DOWN) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
        _selectedItemIndex += 1;
        if (_selectedItemIndex > (s16)_items.size() - 1) _selectedItemIndex = 0;
        return true;
    }

    if (message.isKeyDown(KEY_UP) || (message.isKeyHeld(KEY_UP) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
        _selectedItemIndex -= 1;
        if (_selectedItemIndex < 0) _selectedItemIndex = (s16)_items.size() - 1;
        return true;
    }

    return false;
}

bool cPopMenu::processTouchMessage(cTouchMessage message) {
    cPoint pos = message.position();
    // TODO: Fix it
    if (message.up()) {
        if (windowBelow(pos) == NULL) {
            hide();
            _skipTouch = false;
            return true;
        }

        if (!_skipTouch) {
            hide();
            itemClicked(_selectedItemIndex);
        }

        return true;
    }

    if (message.down() || message.move()) {
        s32 item = itemBelowPoint(pos);
        if (item == -1) {
            _skipTouch = true;
        } else {
            _selectedItemIndex = item;
        }

        return true;
    }

    return false;
}

s32 cPopMenu::itemBelowPoint(const cPoint& pt) {
    cPoint menuPos(position().x + _barLeft, position().y + _itemTopLeftPoint.y - 2);
    cSize menuSize(size().x - _barLeft, _itemHeight * _items.size());
    cRect rect(menuPos, menuPos + menuSize);

    if (rect.surrounds(pt)) {
        s32 item = (pt.y - menuPos.y) / _itemHeight;
        if (item >= (s32)_items.size()) {
            item = _items.size() - 1;
        }

        return item;
    }

    return -1;
}

void cPopMenu::onShow() {
    _selectedItemIndex = 0;
}

}  // namespace akui
