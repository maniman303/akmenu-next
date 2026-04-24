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

    void cWindowManager::cleanModals() {
        auto it = _modals.begin();

        while (it != _modals.end()) {
            cWindow* modal = *it;

            if (windowManager().containsWindow(modal)) {
                ++it;
                continue;
            }

            if (modal->hasFocus()) {
                logger().error("Modal is active.");
                windowManager().setFocusedWindow(windowManager().currentWindow());
            }

            it = _modals.erase(it); // remove safely

            if (modal != NULL) {
                delete modal;
            }
        }
    }

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
        if (aWindow != NULL && aWindow->canRenderBackdrop()) {
            aWindow->scheduleBackdrop();
        }
        updateBackground(false);
        return *this;
    }

    cWindowManager& cWindowManager::addModal(cWindow* aWindow) {
        if (aWindow == NULL) {
            return *this;
        }
        
        _modals.push_back(aWindow);
        addWindow(aWindow);
        aWindow->show();

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
                if (_currentWindow.window()->canRenderBackdrop()) {
                    _currentWindow.window()->scheduleBackdrop();
                }
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
        if (_currentWindow.window() != NULL) {
            _currentWindow.window()->update();
            if (_currentWindow.window()->canRenderBackdrop() && _currentWindow.window()->shouldRenderBackdrop()) {
                gdi().setMainEngineLayer(MEL_MIDDLE);
                _currentWindow.window()->renderBackdrop();
                gdi().setMainEngineLayer(MEL_UP);
                _currentWindow.window()->onRenderBackdrop();
            }

            _currentWindow.window()->render();
        }

        cleanModals();
        
        return *this;
    }

    const cWindowManager& cWindowManager::updateBackground(bool includeCurrent) {
        gdi().setMainEngineLayer(MEL_DOWN);

        for (cWindows::iterator it = _backgroundWindows.begin(); it != _backgroundWindows.end(); ++it) {
            if ((*it).window()->canRenderBackdrop()) {
                (*it).window()->renderBackdrop();
            }
            (*it).window()->render();
        }

        if (includeCurrent && _currentWindow.window()) {
            if (_currentWindow.window()->canRenderBackdrop()) {
                _currentWindow.window()->renderBackdrop();
            }
            _currentWindow.window()->render();
        }

        gdi().pushMainBackground();
        gdi().setMainEngineLayer(MEL_MIDDLE);

        if (includeCurrent && _currentWindow.window()) {
            if (_currentWindow.window()->canRenderBackdrop()) {
                _currentWindow.window()->renderBackdrop();
            }

            gdi().setMainEngineLayer(MEL_UP);

            _currentWindow.window()->render();
        }

        gdi().setMainEngineLayer(MEL_UP);

        return *this;
    }

    bool cWindowManager::processKeyMessage(cKeyMessage message) {
        if (_currentWindow.window() == NULL) {
            return false;
        }

        if (!_currentWindow.window()->inputState()) {
            return false;
        }

        return _currentWindow.window()->processKeyMessage(message);
    }

    bool cWindowManager::processTouchMessage(cTouchMessage message) {
        if (_currentWindow.window() == NULL) {
            return false;
        }

        if (!_currentWindow.window()->inputState()) {
            return false;
        }

        return _currentWindow.window()->processTouchMessage(message);
    }
}  // namespace akui
