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
#include "../stringtool.h"

namespace akui {
    std::vector<cForm*> cForm::_modals;

    void cForm::cleanModals(cForm* current) {
        auto it = _modals.begin();

        // logger().info("Start modal cleanup.");

        while (it != _modals.end()) {
            cForm* modal = *it;

            if (modal == current || windowManager().containsWindow(modal)) {
                ++it;
                continue;
            }

            if (modal->isActive()) {
                logger().error("Modal is active.");
                windowManager().setFocusedWindow(windowManager().currentWindow());
            }

            it = _modals.erase(it); // remove safely

            if (modal != NULL && modal->isDynamic()) {
                delete modal;
            }
        }

        // logger().info("Finished modal cleanup.");
    }

    cForm::cForm(s32 x, s32 y, u32 w, u32 h, cWindow* parent, const std::string& text)
        : cWindow(parent, text)
    //_renderDesc(NULL)
    {
        setSize(cSize(w, h));
        setRelativePosition(cPoint(x, y));
        _modalRet = -1;
        _isDynamic = false;
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

    bool cForm::shouldRenderBackdrop() {
        bool res = false;
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            res |= (*it)->shouldRenderBackdrop();
        }

        return res;
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
            (*it)->renderBackdrop();
        }
    }

    bool cForm::processKeyMessage(cKeyMessage message) {
        if (!isVisible()) {
            return false;
        }

        // TODO: Run input on focused children, if returns false visit another children
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            cWindow* window = *it;
            if (window->isVisible() && window->processKeyMessage(message)) {
                return true;
            }
        }

        return false;
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

    void cForm::doModal() {
        cleanModals(this);
        windowManager().addWindow(this);
        show();
        // logger().info("Do modal.");
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

    void cForm::centerScreen() {
        setRelativePosition(cPoint((SCREEN_WIDTH - _size.x) / 2, (SCREEN_HEIGHT - _size.y) / 2));
    }

    bool cForm::isActive(void) const {
        bool result = isFocused();
        for (std::list<cWindow*>::const_iterator it = _childWindows.begin(); it != _childWindows.end(); it++) {
            if (result) {
                break;
            }

            result = result || (*it)->isFocused();
        }
        
        return result;
    }

    cWindow& cForm::disableFocus(void) {
        for (std::list<cWindow*>::iterator it = _childWindows.begin(); it != _childWindows.end();
            ++it) {
            (*it)->disableFocus();
        }
        return cWindow::disableFocus();
    }

    bool cForm::isDynamic() {
        return _isDynamic;
    }

    void cForm::setDynamic(bool isDynamic) {
        _isDynamic = isDynamic;
    }

}  // namespace akui
