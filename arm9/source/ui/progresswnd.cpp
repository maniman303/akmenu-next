/*
    progresswnd.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "progresswnd.h"
#include "uisettings.h"
#include "windowmanager.h"
#include "../systemfilenames.h"

namespace akui {
    cProgressWnd::cProgressWnd()  // s32 x, s32 y, u32 w, u32 h, cWindow * parent, const std::string &
                                // text )
        : cForm(0, 0, 0, 0, NULL, ""), _bar(0, 0, 180, 24, this, ""), _tip(0, 0, 180, 20, this, "") {
        setSize(cSize(226, 62));
        setRelativePosition(cPoint(14, 64));
    }

    cProgressWnd::~cProgressWnd() {}

    void cProgressWnd::init() {
        loadAppearance(SFN_PROGRESS_WND_BG);
        addChildWindow(&_bar);
        _bar.setRelativePosition(cPoint(4, 9));
        _bar.setPercent(0);

        addChildWindow(&_tip);
        _tip.setTextColor(uiSettings().formTextColor);
        _tip.setRelativePosition(cPoint(4, _size.y - 24));
        _tip.setSize(cSize(_size.x - 8, 12));
    }

    void cProgressWnd::draw() {
        if (!_isVisible) {
            return;
        }

        if (_background.valid()) {
            gdi().maskBlt(_background.buffer(), position().x, position().y, _background.width(), _background.height(), selectedEngine());
        }

        cForm::draw();
    }

    bool cProgressWnd::processKeyMessage(cKeyMessage message) {
        return false;
    }

    cWindow& cProgressWnd::loadAppearance(const std::string& aFileName) {
        _background = createBMP15FromFile(aFileName);
        _bar.loadAppearance(SFN_PROGRESS_BAR_BG);

        return *this;
    }

    void cProgressWnd::setPercent(u8 percent) {
        _bar.setPercent(percent);
    }

    void cProgressWnd::setTipText(const std::string& tipText) {
        _tip.setText(tipText);
    }

    void cProgressWnd::onShow() {
        _bar.setPercent(0);
        windowManager().addWindow(this);
    }

    void cProgressWnd::onHide() {
        _bar.setPercent(0);
        windowManager().removeWindow(this);
    }
}  // namespace akui
