/*
    windowmanager.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "windowmanager.h"
#include "logger.h"

cWindowManager::cWindowManager()
    : _currentWindow(NULL, NULL),
    _focusedWindow(NULL),
    _windowBelowPen(NULL),
    _capturedWindow(NULL),
    _scheduleBackground(0) {}

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
        focusedWindow()->onLostFocus();
    }
    
    _focusedWindow = aWindow;
    if (aWindow) {
        aWindow->onFocused();
    }
}

cWindowManager& cWindowManager::addWindow(cWindow* aWindow) {
    if (_currentWindow.window() != NULL) {
        _currentWindow._focused = focusedWindow();
        _backgroundWindows.push_back(_currentWindow);
    }

    _currentWindow = cWindowRec(aWindow);
    setFocusedWindow(aWindow);
    aWindow->onDisplayed();
    if (aWindow != NULL && aWindow->canRenderBackdrop()) {
        aWindow->scheduleBackdrop();
    }

    scheduleBackground(false);

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
            _currentWindow.window()->onDisplayed();
            if (_currentWindow.window()->canRenderBackdrop()) {
                _currentWindow.window()->scheduleBackdrop();
            }
        }
    } else {
        for (std::reverse_iterator<cWindows::iterator> it = _backgroundWindows.rbegin(); it != _backgroundWindows.rend(); it++) {
            if (it->window() == aWindow) {
                _backgroundWindows.erase(std::prev(it.base()));
                break;
            }
        }
    }
    
    if (focusedWindow() && aWindow->doesHierarchyContain(focusedWindow())) {
        setFocusedWindow(_currentWindow.window());
    }

    scheduleBackground(true);
    
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

        renderBackground();

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

void cWindowManager::scheduleBackground(bool total) {
    if (total) {
        _scheduleBackground = 2;
    } else if (_scheduleBackground == 0) {
        _scheduleBackground = 1;
    }
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

void cWindowManager::renderBackground() {
    if (_scheduleBackground == 0) {
        return;
    }

    if (_scheduleBackground == 1) {
        updateBackground(false);
        _scheduleBackground = 0;
        return;
    }

    updateBackground(true);
    gdi().presentMain();
    updateBackground(false);

    _scheduleBackground = 0;
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
