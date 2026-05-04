/*
    form.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "form.h"
#include "timer.h"
#include "logger.h"
#include "windowmanager.h"
#include "../globalsettings.h"
#include "../stringtool.h"

namespace akui {
    cForm::cForm(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
        : cWindow(parent, text)
    //_renderDesc(NULL)
    {
        setSize(cSize(w, h));
        setRelativePosition(cPoint(x, y));
        _modalRet = -1;
    }

    cForm::~cForm() {
        // if( _renderDesc )
        //     delete _renderDesc;
    }

    cForm& cForm::addChildWindow(cWindow* aWindow) {
        _childWindows.push_back(aWindow);
        return *this;
    }

    cForm& cForm::removeChildWindow(cWindow* aWindow) {
        _childWindows.remove(aWindow);
        // layouter_->removeWindow(aWindow);
        return *this;
    }

    void cForm::update() {
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            (*it)->update();
        }
    }

    void cForm::onFocused() {
        gs().scrollTick = 0;
    }

    bool cForm::canRenderBackdrop() {
        if (_canRenderBackdrop) {
            return true;
        }

        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            if ((*it)->canRenderBackdrop()) {
                return true;
            }
        }

        return false;
    }

    bool cForm::shouldRenderBackdrop() {
        if (_scheduleBackdrop) {
            return true;
        }

        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            if ((*it)->shouldRenderBackdrop()) {
                return true;
            }
        }

        return false;
    }

    void cForm::onRenderBackdrop() {
        _scheduleBackdrop = false;

        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            (*it)->onRenderBackdrop();
        }
    }

    void cForm::draw() {
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            (*it)->render();
        }
    }

    void cForm::drawBackdrop() {
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            if ((*it)->canRenderBackdrop()) {
                (*it)->renderBackdrop();
            }
        }
    }

    cWindow* cForm::findChildXAxis(cPoint current, s16 direction) {
        cWindow* res = NULL;
        s32 minScore = INT32_MAX;

        for (cWindow* child : _childWindows) {
            if (!child->isFocusable() || !child->isVisible()) {
                continue;
            }

            cPoint position = child->focusRectangle().center();

            if (position.x * direction <= current.x * direction) {
                continue;
            }

            s32 score = std::abs(current.x - position.x) + std::abs(current.y - position.y) * std::abs(current.y - position.y);
            if (score >= minScore) {
                continue;
            }

            res = child;
            minScore = score;
        }

        return res;
    }

    cWindow* cForm::findChildYAxis(cPoint current, s16 direction) {
        cWindow* res = NULL;
        s32 minScore = INT32_MAX;

        for (cWindow* child : _childWindows) {
            if (!child->isFocusable() || !child->isVisible() || child->size() == cSize(0, 0)) {
                continue;
            }

            cPoint position = child->focusRectangle().center();

            if (position.y * direction <= current.y * direction) {
                continue;
            }

            s32 score = std::abs(current.y - position.y) + std::abs(current.x - position.x) * std::abs(current.x - position.x);
            if (score >= minScore) {
                continue;
            }

            res = child;
            minScore = score;
        }

        return res;
    }

    bool cForm::processKeyMessage(cKeyMessage message) {
        if (!isVisible()) {
            return false;
        }

        cWindow* focused = NULL;
        for (cWindow* child : _childWindows) {
            if (child->isFocused()) {
                focused = child;
                break;
            }
        }

        if (focused == NULL) {
            return false;
        }

        bool focusedRes = focused->isVisible() ? focused->processKeyMessage(message) : false;
        if (focusedRes) {
            return true;
        }

        if (message.isKeyDown(KEY_DOWN)) {
            focused = findChildYAxis(focused->focusRectangle().center(), 1);
        } else if (message.isKeyDown(KEY_UP)) {
            focused = findChildYAxis(focused->focusRectangle().center(), -1);
        } else if (message.isKeyDown(KEY_LEFT)) {
            focused = findChildXAxis(focused->focusRectangle().center(), -1);
        } else if (message.isKeyDown(KEY_RIGHT)) {
            focused = findChildXAxis(focused->focusRectangle().center(), 1);
        }

        if (focused == NULL || windowManager().focusedWindow() == focused) {
            return false;
        }

        setFocusedChild(focused);

        return true;
    }

    bool cForm::processTouchMessage(cTouchMessage message) {
        if (!isVisible()) {
            return false;
        }

        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            cWindow* window = *it;
            if (window->isVisible() && window->processTouchMessage(message)) {
                windowManager().setFocusedWindow(window, true);

                nocashMessage(formatString("(%s) processed touch message.", window->text().c_str()).c_str());
                return true;
            }
        }

        return false;
    }

    cRect cForm::focusRectangle() const {
        for (cWindow* child : _childWindows) {
            if (child->isVisible() && child->isFocused()) {
                return child->focusRectangle();
            }
        }

        return cWindow::focusRectangle();
    }

    cWindow* cForm::windowBelow(const cPoint& p) {
        cWindow* ret = cWindow::windowBelow(p);  // 先看自己在不在点下面
        if (ret == NULL) {
            return NULL;
        }

        std::list<cWindow*>::reverse_iterator it;
        for (it = _childWindows.rbegin(); it != _childWindows.rend(); ++it) {
            cWindow* window = *it;
            cWindow* cw = window->windowBelow(p);
            // dbg_printf( "check child (%s)\n", window->text().c_str() );
            if (cw != NULL) {
                return cw;
            }
        }

        return NULL;
    }

    u32 cForm::modalRet() {
        return _modalRet;
    }

    void cForm::onOK() {
        _modalRet = 1;
        windowManager().removeWindow(this);
        hide();

        if (onAccepted) {
            onAccepted();
        }
    }

    void cForm::onCancel() {
        _modalRet = 0;
        windowManager().removeWindow(this);
        hide();

        if (onRejected) {
            onRejected();
        }
    }

    void cForm::setFocusedChild(cWindow* child) {
        windowManager().setFocusedWindow(child);
    }

    void cForm::centerScreen() {
        setRelativePosition(cPoint((SCREEN_WIDTH - _size.x) / 2, (SCREEN_HEIGHT - _size.y) / 2));
    }

    bool cForm::hasFocus() const {
        if (isFocused()) {
            return true;
        }

        for (std::list<cWindow*>::const_iterator it = _childWindows.begin(); it != _childWindows.end(); it++) {
            if ((*it)->isFocused()) {
                return true;
            }
        }
        
        return false;
    }
}  // namespace akui
