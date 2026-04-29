#include "subwindowmanager.h"
#include "../analogclock.h"
#include "../bigclock.h"
#include "../calendar.h"
#include "../calendarwnd.h"
#include "../topbar.h"

cSubWindowManager::cSubWindowManager() {}

cSubWindowManager::~cSubWindowManager() {}

void cSubWindowManager::init() {
    calendarWnd().init();
    calendar().init();
    analogClock().init();
    bigClock().init();
    topbar().init();
}

void cSubWindowManager::blink() {
    bigClock().blinkColon();
    topbar().blink();
}

void cSubWindowManager::update() {
    updateBackground();

    topbar().update();

    analogClock().render();
    bigClock().render();
    topbar().render();
}

void cSubWindowManager::updateBackground() {
    bool drawBg = false;

    drawBg |= calendarWnd().shouldRenderBackdrop();
    drawBg |= calendar().shouldRenderBackdrop();

    if (!drawBg) {
        return;
    }

    gdi().setSubEngineLayer(SEL_DOWN);
    
    calendarWnd().renderBackdrop();
    calendarWnd().onRenderBackdrop();
    calendar().renderBackdrop();
    calendar().onRenderBackdrop();

    gdi().setSubEngineLayer(SEL_UP);
    gdi().pushSubBackground();
}