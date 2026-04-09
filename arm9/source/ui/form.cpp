/*
    form.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "form.h"
#include "timer.h"
#include "ui.h"
#include "logger.h"
//#include "dbgtool.h"
//#include "windowmanager.h"

namespace akui {
    std::vector<cForm*> cForm::_modals;

    void cForm::cleanModals(cForm* current) {
        std::vector<cForm*> modalsToRemove;

        for (size_t i = 0; i < _modals.size(); i++) {
            cForm* modal = _modals[i];
            if (modal == current || windowManager().containsWindow(modal)) {
                continue;
            }

            modalsToRemove.push_back(modal);
        }

        for (size_t i = 0; i < modalsToRemove.size(); i++) {
            cForm* modal = modalsToRemove[i];
            _modals.erase(std::remove(_modals.begin(), _modals.end(), modal), _modals.end());

            if (modal != NULL && modal->isDynamic()) {
                delete modal;
            }
        }
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

    void cForm::draw() {
        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            (*it)->render();
        }
    }

    bool cForm::processKeyMessage(cKeyMessage message) {
        if (!isVisible()) {
            return false;
        }

        bool ret = false;

        // TODO: Run input on visited children, if returns false visit another children

        return ret;
    }

    bool cForm::processTouchMessage(cTouchMessage message) {
        if (!isVisible()) {
            return false;
        }

        bool ret = false;

        std::list<cWindow*>::iterator it;
        for (it = _childWindows.begin(); it != _childWindows.end(); ++it) {
            cWindow* window = *it;
            ret = window->processTouchMessage(message);
            if (ret) {
                nocashMessage(formatString("(%s) processed touch message.", window->text().c_str()).c_str());
                break;
            }
        }

        return ret;
    }

    cWindow* cForm::windowBelow(const cPoint& p) {
        cWindow* ret = cWindow::windowBelow(p);  // 先看自己在不在点下面

        if (ret != 0) {
            std::list<cWindow*>::reverse_iterator it;
            for (it = _childWindows.rbegin(); it != _childWindows.rend(); ++it) {
                cWindow* window = *it;
                cWindow* cw = window->windowBelow(p);
                // dbg_printf( "check child (%s)\n", window->text().c_str() );
                if (cw != 0) {
                    ret = cw;
                    break;
                }
            }
        }

        return ret;
    }

    u32 cForm::modalRet() {
        return _modalRet;
    }

    void cForm::doModal() {
        cleanModals(this);
        show();
        windowManager().addWindow(this);
        // logger().info("Do modal.");
    }

    void cForm::onOK() {
        _modalRet = 1;
        hide();

        if (onAccepted) {
            onAccepted();
        }

        windowManager().removeWindow(this);
    }

    void cForm::onCancel() {
        _modalRet = 0;
        hide();

        if (onRejected) {
            onRejected();
        }

        windowManager().removeWindow(this);
    }

    void cForm::centerScreen() {
        setRelativePosition(cPoint((SCREEN_WIDTH - _size.x) / 2, (SCREEN_HEIGHT - _size.y) / 2));
    }

    bool cForm::isActive(void) const {
        bool result = isFocused();
        for (std::list<cWindow*>::const_iterator it = _childWindows.begin();
            !result && it != _childWindows.end(); ++it) {
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
