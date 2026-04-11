/*
    msgbox.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "msgbox.h"
#include "fontfactory.h"
#include "language.h"
#include "ui.h"

namespace akui {
    void cMessageBox::showModal(const std::string& title, const std::string& msg, u32 style) {
        showModal(title, msg, {}, {});
    }

    void cMessageBox::showModal(const std::string& title, const std::string& msg, u32 style, std::function<void()> onClosed) {
        showModal(title, msg, style, onClosed, onClosed);
    }

    void cMessageBox::showModal(const std::string& title, const std::string& msg, u32 style,
        std::function<void()> onAccepted, std::function<void()> onRejected) {
        cMessageBox* modal = new cMessageBox(title, msg, style);
        modal->setDynamic(true);
        modal->onAccepted = onAccepted;
        modal->onRejected = onRejected;
        modal->doModal();

        _modals.push_back(modal);
    }

    cMessageBox::cMessageBox(const std::string& title, const std::string& msg, u32 style)
        : cMessageBox(12, 36, 232, 120, title, msg, style) {}

    cMessageBox::cMessageBox(s32 x, s32 y, u32 w, u32 h, const std::string& title,
                            const std::string& msg, u32 style)
        : cForm(x, y, w, h, NULL, title) {
        std::string breakedMsg = font().BreakLine(msg, 192);
        u32 largestLineWidth = font().TextWidth(breakedMsg);
        size_t lineCount = linesInString(breakedMsg);

        _size.x = largestLineWidth + 48;
        if (_size.x < 192) _size.x = 192;
        if (_size.x > 256) _size.x = 256;
        if (_size.y > 192) _size.y = 192;
        if (_size.x & 1) --_size.x;  // 4 byte align, for speed optimization
        _size.y = lineCount * font().GetHeight() + 60;

        s32 centerX = (SCREEN_WIDTH - _size.x) / 2;
        if (centerX & 1) centerX--;
        s32 centerY = (SCREEN_HEIGHT - _size.y) / 2;
        setRelativePosition(cPoint(centerX, centerY));

        _textPoision.x = position().x + (size().x - largestLineWidth) / 2;
        _textPoision.y = position().y;
        dbg_printf("_size.x %d largestLineWidth %d\n", _size.x, largestLineWidth);

        _text = title;
        _msg = breakedMsg;
        _style = style;
        //_msgRet = -1;
        _buttonOK = NULL;
        _buttonCANCEL = NULL;
        _buttonYES = NULL;
        _buttonNO = NULL;

        _buttonOK = new cButton(0, 0, 46, 18, this, "\x01 OK");
        _buttonOK->setText("\x01 " + LANG("message box", "ok"));
        _buttonOK->setStyle(cButton::press);
        _buttonOK->hide();

        _buttonOK->loadAppearance(SFN_BUTTON3);
        _buttonOK->setStyle(cButton::press);
        _buttonOK->clicked.connect(this, &cMessageBox::onOK);
        addChildWindow(_buttonOK);

        _buttonCANCEL = new cButton(0, 0, 46, 18, this, "\x02 Cancel");
        _buttonCANCEL->setText("\x02 " + LANG("message box", "cancel"));
        _buttonCANCEL->setStyle(cButton::press);
        _buttonCANCEL->hide();
        _buttonCANCEL->loadAppearance(SFN_BUTTON3);
        _buttonCANCEL->clicked.connect(this, &cMessageBox::onCancel);
        addChildWindow(_buttonCANCEL);

        _buttonYES = new cButton(0, 0, 46, 18, this, "\x01 Yes");
        _buttonYES->setText("\x01 " + LANG("message box", "yes"));
        _buttonYES->setStyle(cButton::press);
        _buttonYES->hide();
        _buttonYES->loadAppearance(SFN_BUTTON3);
        _buttonYES->clicked.connect(this, &cMessageBox::onOK);
        addChildWindow(_buttonYES);

        _buttonNO = new cButton(0, 0, 46, 18, this, "\x02 No");
        _buttonNO->setText("\x02 " + LANG("message box", "no"));
        _buttonNO->setStyle(cButton::press);
        _buttonNO->hide();
        //_buttonNO->setTextColor( RGB15(20,14,0) );
        _buttonNO->loadAppearance(SFN_BUTTON3);
        _buttonNO->clicked.connect(this, &cMessageBox::onCancel);
        addChildWindow(_buttonNO);

        s16 nextButtonX = size().x;
        s16 buttonPitch = 60;
        s16 buttonY = size().y - _buttonNO->size().y - 4;
        // 下一个要画的按钮的位置
        if (_style & MB_NO) {
            // 在nextButtonX位置画 NO 按钮
            // nextButtonX -= 按钮宽度 + 空白区宽度
            buttonPitch = _buttonNO->size().x + 8;
            nextButtonX -= buttonPitch;
            _buttonNO->setRelativePosition(cPoint(nextButtonX, buttonY));
            _buttonNO->show();
        }

        if (_style & MB_YES) {
            // 在nextButtonX位置画 YES 按钮
            // nextButtonX -= 按钮宽度 + 空白区宽度
            buttonPitch = _buttonYES->size().x + 8;
            nextButtonX -= buttonPitch;
            _buttonYES->setRelativePosition(cPoint(nextButtonX, buttonY));
            _buttonYES->show();
        }

        if (_style & MB_CANCEL) {
            // 在nextButtonX位置画 CANCEL 按钮
            // nextButtonX -= 按钮宽度 + 空白区宽度
            buttonPitch = _buttonCANCEL->size().x + 8;
            nextButtonX -= buttonPitch;
            _buttonCANCEL->setRelativePosition(cPoint(nextButtonX, buttonY));
            _buttonCANCEL->show();
        }

        if (_style & MB_OK) {
            // 在nextButtonX位置画 OK 按钮
            // nextButtonX -= 按钮宽度 + 空白区宽度
            buttonPitch = _buttonOK->size().x + 8;
            nextButtonX -= buttonPitch;
            _buttonOK->setRelativePosition(cPoint(nextButtonX, buttonY));
            _buttonOK->show();
        }

        if (_style & MB_NONE) {
            buttonPitch = _buttonCANCEL->size().x + 8;
            nextButtonX -= buttonPitch;
        }

        loadAppearance("");
    }

    cMessageBox::~cMessageBox() {
        delete _buttonOK;
        delete _buttonCANCEL;
        delete _buttonYES;
        delete _buttonNO;
    }

    void cMessageBox::onOK() {
        cForm::onOK();
    }

    void cMessageBox::onCancel() {
        cForm::onCancel();
    }

    bool cMessageBox::processKeyMessage(cKeyMessage message) {
        if (message.isKeyUp(KEY_A)) {
            onOK();
            return true;
        }

        if (message.isKeyUp(KEY_B)) {
            onCancel();
            return true;
        }

        return false;
    }

    bool cMessageBox::processTouchMessage(cTouchMessage message) {
        return cForm::processTouchMessage(message);
    }

    void cMessageBox::draw() {
        _renderDesc.draw(windowRectangle(), _engine);
        cForm::draw();

        // draw message text
        int titleOffset = _renderDesc.titleSize().y + 8;
        gdi().setPenColor(uiSettings().formTextColor, _engine);
        gdi().textOut(_textPoision.x, _textPoision.y + titleOffset, _msg.c_str(), _engine);
    }

    cWindow& cMessageBox::loadAppearance(const std::string& aFileName) {
        _renderDesc.loadData(SFN_FORM_TITLE_L, SFN_FORM_TITLE_R, SFN_FORM_TITLE_M);

        _renderDesc.setTitleText(_text);
        return *this;
    }

}  // namespace akui
