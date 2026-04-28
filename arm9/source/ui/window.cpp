/*
    window.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "window.h"
#include "windowmanager.h"

cWindow::cWindow(cWindow* aParent, const std::string& aText)
    : _parent(aParent),
      _text(aText),
      _size(cSize(0, 0)),
      _position(cPoint(0, 0)),
      _relative_position(cPoint(0, 0)),
      _isVisible(true),
      _isSizeSetByUser(false),
      _isFocusable(true),
      _ignoreSizeEvent(false),
      _canRenderBackdrop(false),
      _scheduleBackdrop(false),
      _inputState(true),
      _engine(GE_MAIN) {}

cWindow::~cWindow() {
    if (isFocused()) windowManager().setFocusedWindow(NULL);
}

cWindow& cWindow::setWindowRectangle(const cRect& rect) {
    setSize(rect.size());
    setRelativePosition(rect.position());
    return *this;
}

cRect cWindow::windowRectangle() const {
    return cRect(position(), size(), false);
}

cRect cWindow::focusRectangle() const {
    return windowRectangle();
}

bool cWindow::isFocused() const {
    return windowManager().focusedWindow() == this;
}

bool cWindow::hasFocus() const {
    return isFocused();
}

cWindow& cWindow::enableFocused() {
    onFocused();
    return *this;
}

cWindow& cWindow::disableFocused() {
    onLostFocus();
    return *this;
}

cWindow& cWindow::enableInput() {
    _inputState = true;
    return *this;
}

cWindow& cWindow::disableInput() {
    _inputState = false;
    return *this;
}

bool cWindow::inputState() {
    return _inputState;
}

cWindow* cWindow::windowBelow(const cPoint& p) {
    cWindow* ret = NULL;
    if (isVisible()) {
        if (windowRectangle().surrounds(p)) ret = this;
    }
    return ret;
}

cWindow& cWindow::show() {
    _isVisible = true;
    onShow();
    return *this;
}

cWindow& cWindow::hide() {
    _isVisible = false;
    onHide();
    return *this;
}

void cWindow::exit() {
    onExit();
}

bool cWindow::doesHierarchyContain(cWindow* aWindow) const {
    while (aWindow != NULL) {
        if (aWindow == this) {
            return true;
        }

        aWindow = aWindow->parent();
    }

    return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

cWindow* cWindow::topLevelWindow() const {
    cWindow* ret = (cWindow*)(this);
    cWindow* test = ret;
    while (test != 0) {
        ret = test;
        test = ret->parent();
    }
    return ret;
}

bool cWindow::processKeyMessage(cKeyMessage message) {
    return false;
}

bool cWindow::processTouchMessage(cTouchMessage message) {
    return false;
}

void cWindow::render() {
    if (isVisible()) draw();
}

void cWindow::renderBackdrop() {
    if (isVisible()) drawBackdrop();
}

cWindow& cWindow::setSize(const cSize& aSize) {
    _size = aSize;
    if (!_ignoreSizeEvent) {
        _ignoreSizeEvent = true;
        onResize();
        _ignoreSizeEvent = false;
    }
    _isSizeSetByUser = true;
    return *this;
}

cPoint cWindow::position() const {
    cPoint parentPosition = cPoint(0, 0);
    if (_parent != NULL) {
        parentPosition = _parent->position();
    }

    return parentPosition + _relative_position;
}

cWindow& cWindow::setRelativePosition(const cPoint& rPosition) {
    _relative_position = rPosition;
    onMove();
    return *this;
}

cWindow& cWindow::setText(const std::string& aText) {
    _text = aText;
    onTextChanged();
    return *this;
}
