/*
    settingwnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "cachedinifile.h"
#include "settingwnd.h"
#include "globalsettings.h"
#include "language.h"
#include "msgbox.h"
#include "systemfilenames.h"
#include "uisettings.h"
#include "windowmanager.h"
#include "fontfactory.h"
#include "timer.h"
#include "logger.h"
#define TOP_MARGIN 4

using namespace akui;

cSettingWnd* cSettingWnd::createWindow(cWindow* parent, const std::string& text, const std::string& id, std::function<void(cSettingWnd*)> onSaved) {
    cSettingWnd* wnd = new cSettingWnd(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, parent, text, id);
    wnd->setDynamic(true);
    wnd->onSaved = onSaved;
    _modals.push_back(wnd);

    return wnd;
}

cSettingWnd::cSettingWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text, const std::string& id)
    : cForm(x, y, w, h, parent, text),
      _tabSwitcher(0, 0, w, 18, true, this, "spin"),
      _buttonOK(0, 0, 46, 18, this, "\x01 OK"),
      _buttonCancel(0, 0, 48, 18, this, "\x02 Cancel") {
    _tabSwitcher.loadAppearance("");
    _tabSwitcher.changed.connect(this, &cSettingWnd::onItemChanged);
    addChildWindow(&_tabSwitcher);
    _tabSwitcher.selectItem(0);
    _tabSwitcher.hide();
    _tabSwitcher.disableFocus();

    s16 buttonY = size().y - _buttonCancel.size().y - 4;

    _buttonCancel.setStyle(cButton::press);
    _buttonCancel.setText("\x02 " + LANG("setting window", "cancel"));
    _buttonCancel.setTextColor(uis().buttonTextColor);
    _buttonCancel.loadAppearance(SFN_BUTTON3);
    _buttonCancel.clicked.connect(this, &cSettingWnd::onCancel);
    addChildWindow(&_buttonCancel);

    _buttonOK.setStyle(cButton::press);
    _buttonOK.setText("\x01 " + LANG("setting window", "ok"));
    _buttonOK.setTextColor(uis().buttonTextColor);
    _buttonOK.loadAppearance(SFN_BUTTON3);
    _buttonOK.clicked.connect(this, &cSettingWnd::onOK);
    addChildWindow(&_buttonOK);

    s16 nextButtonX = size().x;
    s16 buttonPitch = _buttonCancel.size().x + 8;
    nextButtonX -= buttonPitch;
    _buttonCancel.setRelativePosition(cPoint(nextButtonX, buttonY));

    buttonPitch = _buttonOK.size().x + 8;
    nextButtonX -= buttonPitch;
    _buttonOK.setRelativePosition(cPoint(nextButtonX, buttonY));

    _maxLabelLength = 0;
    _titleOffset = 0;
    _maxTabSize = 0;
    _confirmMessage = LANG("setting window", "confirm text");
    _id = id;
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _spinBoxWidth = ini.GetInt("setting window", "spinBoxWidth", 108);
    _simpleTabs = ini.GetInt("setting window", "simpleTabs", 0);

    loadAppearance("");

    _currentTab = 0;
}

cSettingWnd::~cSettingWnd() {
    for (size_t ii = 0; ii < _tabs.size(); ii++) {
        for (size_t jj = 0; jj < items(ii).size(); jj++) {
            delete items(ii)[jj]._label;
            delete items(ii)[jj]._item;
        }
        delete _tabs[ii]._tab;
    }
}

void cSettingWnd::draw(void) {
    _renderDesc.draw(windowRectangle(), _engine);
    colorLabels();
    cForm::draw();
}

bool cSettingWnd::processKeyMessage(cKeyMessage message) {
    if (message.isKeyDown(KEY_R)) {
        onUIKeyR();
        return true;
    }

    if (message.isKeyDown(KEY_L)) {
        onUIKeyL();
        return true;
    }

    if (message.isKeyUp(KEY_A)) {
        onOK();
        return true;
    }

    if (message.isKeyUp(KEY_B)) {
        onCancel();
        return true;
    }

    if (message.isKeyDown(KEY_LEFT)) {
        onUIKeyLEFT();
        return true;
    }

    if (message.isKeyDown(KEY_RIGHT)) {
        onUIKeyRIGHT();
        return true;
    }

    if (message.isKeyDown(KEY_DOWN) || message.isKeyDown(KEY_UP)) {
        gs().scrollTick = timer().getFrame();
    }

    u32 tickDiff = timer().getFrame() - gs().scrollTick;
    if (message.isKeyDown(KEY_DOWN) || (message.isKeyHeld(KEY_DOWN) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
        onUIKeyDOWN();
        return true;
    }

    if (message.isKeyDown(KEY_UP) || (message.isKeyHeld(KEY_UP) && tickDiff > gs().scrollWait && tickDiff % gs().scrollSpeed == 0)) {
        onUIKeyUP();
        return true;
    }
    
    return false;
}

void cSettingWnd::onOK(void) {
    cMessageBox::showModal(LANG("setting window", "confirm"), _confirmMessage, MB_OK | MB_CANCEL, [this]() {
        if (onSaved) {
            onSaved(this);
        }

        cForm::onOK();
    },
    {});
}

void cSettingWnd::onCancel(void) {
    cForm::onCancel();
}

cWindow& cSettingWnd::loadAppearance(const std::string& aFileName) {
    if (_simpleTabs) {
        _renderDesc.loadData("", "", "");
        _renderDesc.setTitleText("");

        return *this;
    }

    _renderDesc.loadData(SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);
    _renderDesc.setTitleText(_text, true);

    _titleOffset = _renderDesc.titleSize().y;

    _tabSwitcher.setRelativePosition(cPoint(0, _titleOffset));

    return *this;
}

void cSettingWnd::setConfirmMessage(const std::string& text) {
    _confirmMessage = text;
}

void cSettingWnd::addSettingTab(const std::string& text) {
    if (_tabs.size() == 0 && _simpleTabs) {
        _renderDesc.loadData("", "", "");
        _renderDesc.setTitleText("");
        _titleOffset = 0;
    }

    _tabSwitcher.insertItem(text, _tabs.size());
    _tabs.push_back(sSettingTab(new std::vector<sSettingItem>, text));
    _tabSwitcher.show();
}

void cSettingWnd::addSettingItem(const std::string& text, const std::vector<std::string>& itemTexts,
                                 size_t defaultValue) {
    if (0 == itemTexts.size()) return;
    if (defaultValue >= itemTexts.size()) defaultValue = 0;
    if (_maxLabelLength < text.length()) _maxLabelLength = text.length();
    size_t lastTab = _tabs.size() - 1;

    // recompute window size and position
    if (_maxTabSize < (items(lastTab).size() + 1)) {
        _maxTabSize = (items(lastTab).size() + 1);
        setRelativePosition(cPoint((SCREEN_WIDTH - _size.x) / 2, (SCREEN_HEIGHT - _size.y) / 2));
    }

    // insert label, item and set their position
    s32 tabSwitcherHeight = _tabSwitcher.isVisible() ? _tabSwitcher.size().y : 0;
    s32 itemY = (items(lastTab).size() * 20) + static_cast<s32>(_titleOffset) + tabSwitcherHeight + TOP_MARGIN;
    s32 itemX = 8;

    cSpinBox* item = new cSpinBox(0, 0, 108, 18, this, "spin");
    for (size_t ii = 0; ii < itemTexts.size(); ++ii) {
        item->insertItem(itemTexts[ii], ii);
    }

    item->loadAppearance("");
    item->setSize(cSize(_spinBoxWidth, 18));
    item->setRelativePosition(cPoint(_size.x - _spinBoxWidth - 4, itemY));
    item->hide();
    addChildWindow(item);
    item->selectItem(defaultValue);

    cStaticText* label = new cStaticText(0, 0, _maxLabelLength * 6, font().GetHeight(), this, text);
    itemY += (item->windowRectangle().height() - label->windowRectangle().height()) / 2;
    label->setRelativePosition(cPoint(itemX, itemY));
    label->setTextColor(uis().formTextColor);
    label->setSize(cSize(_size.x / 2 + 8, 12));
    label->hide();
    addChildWindow(label);

    items(lastTab).push_back(sSettingItem(label, item));

    // recompute button position
    s16 buttonY = size().y - _buttonCancel.size().y - 4;

    _buttonCancel.setRelativePosition(cPoint(_buttonCancel.relativePosition().x, buttonY));
    _buttonOK.setRelativePosition(cPoint(_buttonOK.relativePosition().x, buttonY));
}

void cSettingWnd::onShow() {
    // logger().info("On show.");
    ShowTab(_currentTab);
}

void cSettingWnd::onGainedFocus() {
    ShowTab(_currentTab);
}

void cSettingWnd::onUIKeyUP(void) {
    ssize_t focusItem = focusedItemId();
    if (--focusItem < 0) focusItem = items(_currentTab).size() - 1;
    windowManager().setFocusedWindow(items(_currentTab)[focusItem]._item);
}

void cSettingWnd::onUIKeyDOWN(void) {
    ssize_t focusItem = focusedItemId();
    if (++focusItem >= (ssize_t)items(_currentTab).size()) focusItem = 0;
    windowManager().setFocusedWindow(items(_currentTab)[focusItem]._item);
}

void cSettingWnd::onUIKeyLEFT(void) {
    cSpinBox* item = focusedItem();
    if (item) item->selectPrev();
}

void cSettingWnd::onUIKeyRIGHT(void) {
    cSpinBox* item = focusedItem();
    if (item) item->selectNext();
}

void cSettingWnd::onUIKeyL(void) {
    _tabSwitcher.selectPrev();
}

void cSettingWnd::onUIKeyR(void) {
    _tabSwitcher.selectNext();
}

ssize_t cSettingWnd::getItemSelection(size_t tabId, size_t itemId) {
    if (tabId >= _tabs.size()) return -1;
    if (itemId >= items(tabId).size()) return -1;
    return items(tabId)[itemId]._item->selectedItemId();
}

/*
0.. - item focused
-1 - something else focused
*/
ssize_t cSettingWnd::focusedItemId(void) {
    ssize_t focusItem = -1;
    for (size_t ii = 0; ii < items(_currentTab).size(); ++ii) {
        if (items(_currentTab)[ii]._item->isActive()) {
            focusItem = ii;
            break;
        }
    }
    return focusItem;
}

cSpinBox* cSettingWnd::focusedItem(void) {
    ssize_t focusItem = focusedItemId();
    if (focusItem >= 0) return items(_currentTab)[focusItem]._item;
    return NULL;
}

void cSettingWnd::colorLabels(void) {
    ssize_t focusItem = focusedItemId();
    ssize_t tabSize = (ssize_t)items(_currentTab).size();
    if (focusItem >= tabSize || focusItem < 0) {
        return;
    }

    for (ssize_t i = 0; i < tabSize; i++) {
        items(_currentTab)[i]._label->setTextColor(i == focusItem ? uis().formTextHighLightColor : uis().formTextColor);
    }
}

void cSettingWnd::HideTab(size_t index) {
    if (index >= _tabs.size()) return;
    for (size_t ii = 0; ii < items(index).size(); ++ii) {
        items(index)[ii]._label->hide();
        items(index)[ii]._item->hide();
    }
}

void cSettingWnd::ShowTab(size_t index) {
    if (index >= _tabs.size()) {
        return;
    }

    // logger().info(formatString("Showing tab %d.", index));

    for (size_t ii = 0; ii < items(index).size(); ++ii) {
        items(index)[ii]._label->show();
        items(index)[ii]._item->show();
    }

    _tabSwitcher.setPrefixIcon(SFN_UI_PREFIXES_DIRECTORY + prefixIconName(index));

    if (items(index).size()) windowManager().setFocusedWindow(items(index)[0]._item);
}

void cSettingWnd::SwitchTab(size_t oldIndex, size_t newIndex) {
    HideTab(oldIndex);
    ShowTab(newIndex);
}

void cSettingWnd::onItemChanged(akui::cSpinBox* item) {
    size_t newTab = item->selectedItemId();
    SwitchTab(_currentTab, newTab);
    _currentTab = newTab;
}
