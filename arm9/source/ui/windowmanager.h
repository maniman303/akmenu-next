/*
    windowmanager.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <list>
#include <string>
#include "userinput.h"
#include "window.h"
#include "keymessage.h"
#include "touchmessage.h"

class cWindowManager {
  private:
    struct cWindowRec {
        cWindow* _window;
        cWindow* _focused;
        cWindowRec(cWindow* window, cWindow* focused = NULL) : _window(window), _focused(focused) {}
        cWindow* window() const { return _window; }
    };
    typedef std::list<cWindowRec> cWindows;

  private:
    void cleanModals();
    
    cWindows _backgroundWindows;
    cWindowRec _currentWindow;
    cWindow* _focusedWindow;
    cWindow* _windowBelowPen;
    cWindow* _capturedWindow;  // process touch for non-focusable window
    std::vector<cWindow*> _modals;
  protected:
    cWindow* windowBelowPen(void) const { return _windowBelowPen; }
    const cWindowManager& updateBackground(bool includeCurrent);

  public:
    cWindowManager();
    ~cWindowManager();
    cWindow* focusedWindow(void) const { return _focusedWindow; }
    void setFocusedWindow(cWindow* aWindow);
    void setFocusedWindow(cWindow* aWindow, bool isTouch);
    cWindowManager& addWindow(cWindow* aWindow);
    cWindowManager& addModal(cWindow* aWindow);
    cWindowManager& removeWindow(cWindow* aWindow);
    cWindow* currentWindow(void) const { return _currentWindow.window(); }
    bool containsWindow(cWindow* aWindow);
    const cWindowManager& update(void);
    bool processKeyMessage(cKeyMessage message);
    bool processTouchMessage(cTouchMessage message);
};

typedef t_singleton<cWindowManager> cWindowManager_s;
inline cWindowManager& windowManager(void) {
    return cWindowManager_s::instance();
}
