/*
    settingwnd.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <string>
#include <functional>
#include "form.h"
#include "formdesc.h"
#include "message.h"
#include "spinbox.h"
#include "statictext.h"

class cSettingWnd : public akui::cForm {
  public:
    cSettingWnd(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text, const std::string& id);
    ~cSettingWnd();

  public:
    static cSettingWnd* createWindow(cWindow* parent, const std::string& text, const std::string& id, std::function<void(cSettingWnd*)> onSaved);

    std::function<void(cSettingWnd*)> onSaved;

    void draw(void);
    bool process(const akui::cMessage& msg);
    cWindow& loadAppearance(const std::string& aFileName);
    void addSettingTab(const std::string& text);
    void addSettingItem(const std::string& text, const std::vector<std::string>& itemTexts,
                        size_t defaultValue);
    ssize_t getItemSelection(size_t tabId, size_t itemId);
    void setConfirmMessage(const std::string& text);

  protected:
    struct sSettingItem {
        akui::cStaticText* _label;
        akui::cSpinBox* _item;
        sSettingItem(akui::cStaticText* label, akui::cSpinBox* item) : _label(label), _item(item){};
    };
    struct sSettingTab {
        std::vector<sSettingItem>* _tab;
        std::string _title;
        sSettingTab(std::vector<sSettingItem>* tab, const std::string& title)
            : _tab(tab), _title(title) {};
    };

  protected:
    void onOK(void) override;
    void onCancel(void) override;
    void onShow() override;
    void onUIKeyUP(void);
    void onUIKeyDOWN(void);
    void onUIKeyLEFT(void);
    void onUIKeyRIGHT(void);
    void onUIKeyL(void);
    void onUIKeyR(void);
    bool processKeyMessage(const akui::cKeyMessage& msg);
    void onItemChanged(akui::cSpinBox* item);
    ssize_t focusedItemId(void);
    akui::cSpinBox* focusedItem(void);
    void colorLabels(void);

    void HideTab(size_t index);
    void ShowTab(size_t index);
    void SwitchTab(size_t oldIndex, size_t newIndex);

    std::vector<sSettingItem>& items(size_t index) { return *_tabs[index]._tab; };

    std::string prefixIconName(size_t index) { return _id + "_" + std::to_string(index) + ".bmp"; }

    u32 _titleOffset;
    u32 _maxLabelLength;
    u32 _spinBoxWidth;
    bool _simpleTabs;
    std::vector<sSettingTab> _tabs;
    size_t _currentTab;
    size_t _maxTabSize;
    std::string _confirmMessage;
    std::string _id;

    akui::cSpinBox _tabSwitcher;
    akui::cButton _buttonOK;
    akui::cButton _buttonCancel;
    akui::cFormDesc _renderDesc;
};
