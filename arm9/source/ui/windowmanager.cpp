/*
    windowmanager.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "windowmanager.h"
#include "logger.h"

namespace akui {
    cWindowManager::cWindowManager()
        : _currentWindow(NULL, NULL),
        _focusedWindow(NULL),
        _windowBelowPen(NULL),
        _capturedWindow(NULL) {}

    cWindowManager::~cWindowManager() {}

    void cWindowManager::setFocusedWindow(cWindow* aWindow) {
        setFocusedWindow(aWindow, false);
    }

    void cWindowManager::setFocusedWindow(cWindow* aWindow, bool isTouch) {
        if (aWindow == NULL || aWindow == focusedWindow() || !aWindow->isFocusable()) {
            return;
        }

        if (isTouch && !aWindow->isTouchFocusable()) {
            return;
        }

        if (focusedWindow() != NULL) {
            focusedWindow()->disableFocused();
        }
        
        _focusedWindow = aWindow;
        if (aWindow) {
            aWindow->enableFocused();
        }
    }

    cWindowManager& cWindowManager::addWindow(cWindow* aWindow) {
        if (_currentWindow.window() != NULL) {
            _currentWindow._focused = focusedWindow();
            _backgroundWindows.push_back(_currentWindow);
        }
        _currentWindow = cWindowRec(aWindow);
        setFocusedWindow(aWindow);
        updateBackground(false);
        return *this;
    }

    cWindowManager& cWindowManager::removeWindow(cWindow* aWindow) {
        cWindow* current = _currentWindow.window();
        if (current != NULL && current->doesHierarchyContain(aWindow)) {
            if (_backgroundWindows.empty()) {
                _currentWindow = cWindowRec(NULL, NULL);
            } else {
                _currentWindow = _backgroundWindows.back();
                _backgroundWindows.pop_back();
                setFocusedWindow(_currentWindow._focused);
            }
        } else {
            for (cWindows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end();
                ++it) {
                if ((*it).window() == aWindow) {
                    _backgroundWindows.erase(it);
                    break;
                }
            }
        }
        
        if (focusedWindow() && aWindow->doesHierarchyContain(focusedWindow())) {
            // logger().info("Hierarchy includes focused window.");
            _focusedWindow = _currentWindow.window();
        }

        updateBackground(true);
        gdi().presentMain();
        updateBackground(false);
        
        return *this;
    }

    bool cWindowManager::containsWindow(cWindow* aWindow) {
        if (aWindow == NULL) {
            return false;
        }

        if (_currentWindow.window() == aWindow) {
            return true;
        }

        for (cWindows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it) {
            if ((*it).window() == aWindow) {
                return true;
            }
        }

        return false;
    }

    const cWindowManager& cWindowManager::update(void) {
    #if 0
        for(cWindows::iterator it=_backgroundWindows.begin();it!=_backgroundWindows.end();++it)
        {
        dbg_printf("background (%s)\n",(*it)()->text().c_str());
        }
        dbg_printf("currentWindow (%s)\n",_currentWindow()?_currentWindow()->text().c_str():"NULL");
    #endif
        if (_currentWindow.window() != NULL) {
            _currentWindow.window()->update();
            _currentWindow.window()->render();
        }
        return *this;
    }

    const cWindowManager& cWindowManager::updateBackground(bool includeCurrent) {
        gdi().setMainEngineLayer(MEL_DOWN);

        for (cWindows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it) {
            (*it).window()->render();
        }

        if (includeCurrent && _currentWindow.window()) {
            _currentWindow.window()->render();
        }

        gdi().setMainEngineLayer(MEL_UP);
        gdi().pushMainBackground();

        if (includeCurrent && _currentWindow.window()) {
            _currentWindow.window()->render();
        }

        return *this;
    }

    bool cWindowManager::processKeyMessage(cKeyMessage message) {
        if (_currentWindow.window() == NULL) {
            return false;
        }

        return _currentWindow.window()->processKeyMessage(message);
    }

    bool cWindowManager::processTouchMessage(cTouchMessage message) {
        if (_currentWindow.window() == NULL) {
            return false;
        }

        return _currentWindow.window()->processTouchMessage(message);
    }
}  // namespace akui
