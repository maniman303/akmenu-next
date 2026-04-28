/*
    popmenu.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

//�

#include "popmenu.h"
#include "fontfactory.h"
#include "timer.h"
#include "logger.h"
#include "uisettings.h"
#include "windowmanager.h"
#include "../globalsettings.h"

namespace akui {

    cPopMenu::cPopMenu(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
        : cWindow(parent, text) {
        setSize(cSize(w, h));
        setRelativePosition(cPoint(x, y));

        _selectedItemIndex = -1;
        _itemHeight = 0;
        _itemWidth = 0;
        _barLeft = 2;

        _textColor = uiSettings().popMenuTextColor;
        _textHighLightColor = uiSettings().popMenuTextHighLightColor;
        _barColor = uiSettings().popMenuBarColor;

        _skipTouch = false;
    }

    cPopMenu::~cPopMenu() { }

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

    cWindow& cPopMenu::loadAppearance(const std::string& aFileName) {
        _background = createBMP15FromFile(aFileName);

        return *this;
    }

    void cPopMenu::draw() {
        if (_background.valid()) {
            gdi().maskBlt(_background.buffer(), position().x, position().y, _background.width(), _background.height(), selectedEngine());
        }
        
        drawItems();
    }

    void cPopMenu::drawItems() {
        s16 offsetY = 0;

        for (size_t i = 0; i < _items.size(); i++) {
            std::string text = _items[i];
            if (text.empty()) {
                offsetY -= _itemHeight;
                continue;
            }

            s16 itemX = position().x + _itemTopLeftPoint.x;
            s16 itemY = position().y + i * _itemHeight + _itemTopLeftPoint.y + offsetY;
            if (_selectedItemIndex == (s16)i) {
                s16 barX = position().x + _barLeft;
                s16 barY = itemY - 2;
                gdi().fillRect(_barColor, _barColor, barX, barY, barWidth(), _itemHeight, _engine);
                gdi().setPenColor(_textHighLightColor, _engine);
            } else {
                gdi().setPenColor(_textColor, _engine);
            }

            gdi().textOut(itemX, itemY, text.c_str(), _engine, fontSecondary());
        }
    }

    s16 cPopMenu::barWidth(void) {
        return _itemWidth ? _itemWidth : (_size.x - 2 * _barLeft);
    }

    bool cPopMenu::processKeyMessage(cKeyMessage message) {
        if (message.isKeyUp(KEY_A)) {
            if (_selectedItemIndex != -1) {
                itemClicked(_selectedItemIndex);
            }

            exit();
            return true;
        }

        if (message.isKeyUp(KEY_B)) {
            exit();
            return true;
        }

        if (message.isKeyDown(KEY_DOWN) || message.isKeyDown(KEY_UP)) {
            gs().scrollTick = timer().getFrame();
        }

        if (gs().scrollTick == 0) {
            return false;
        }

        u32 tickDiff = timer().getFrame() - gs().scrollTick;
        if (message.isKeyDown(KEY_DOWN) || (message.isKeyHeld(KEY_DOWN) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
            selectItem(_selectedItemIndex + 1, false);
            return true;
        }

        if (message.isKeyDown(KEY_UP) || (message.isKeyHeld(KEY_UP) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
            selectItem(_selectedItemIndex - 1, false);
            return true;
        }

        return false;
    }

    bool cPopMenu::processTouchMessage(cTouchMessage message) {
        cPoint pos = message.position();
        if (message.up()) {
            if (windowBelow(pos) == NULL) {
                exit();
            } else if (!_skipTouch) {
                if (_selectedItemIndex != -1) {
                    itemClicked(_selectedItemIndex);
                }

                exit();
            }

            _skipTouch = false;

            return true;
        }

        if (message.down() || message.move()) {
            s32 item = itemBelowPoint(pos);
            if (item == -1) {
                _skipTouch = true;
            } else {
                _skipTouch = false;
                selectItem(item, false);
            }

            return true;
        }

        return false;
    }

    void cPopMenu::selectItem(s16 item, bool silent) {
        if (_items.empty()) {
            _selectedItemIndex = 0;
            return;
        }

        logger().info("Selected: " + std::to_string(item));

        if (item == _selectedItemIndex) {
            bool allEmpty = true;
            for (std::string& text : _items) {
                if (!text.empty()) {
                    allEmpty = false;
                    break;
                }
            }

            if (!allEmpty && _items[item].empty()) {
                selectItem(item + 1, silent);
            }

            return;
        }

        bool next = item > _selectedItemIndex;
        if (item < 0) {
            item = static_cast<s16>(_items.size()) - 1;
        } else if (item >= static_cast<s16>(_items.size())) {
            item = 0;
        }

        if (!_items[item].empty()) {
            _selectedItemIndex = item;
            
            if (!silent) {
                itemSelected(_selectedItemIndex);
            }

            return;
        }

        return selectItem(item + (next ? 1 : -1), silent);
    }

    s32 cPopMenu::itemBelowPoint(const cPoint& pt) {
        cPoint menuPos(position().x + _barLeft, position().y + _itemTopLeftPoint.y - 2);
        cSize menuSize(size().x - _barLeft, _itemHeight * _items.size());
        cRect rect(menuPos, menuPos + menuSize);

        if (rect.surrounds(pt)) {
            s32 item = (pt.y - menuPos.y) / _itemHeight;

            s32 itemOffset = 0;
            for (int i = 0; i <= item; i++) {
                if (_items[i].empty()) {
                    itemOffset++;
                }
            }

            if (item + itemOffset >= (s32)_items.size()) {
                return -1;
            }

            return item + itemOffset;
        }

        return -1;
    }

    void cPopMenu::onShow() {
        selectItem(0, true);
    }

    void cPopMenu::onExit() {
        menuExit();
        windowManager().removeWindow(this);
    }

}  // namespace akui
