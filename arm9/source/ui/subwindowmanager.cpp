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

    analogClock().draw();
    bigClock().draw();

    topbar().draw();
}

void cSubWindowManager::updateBackground() {
    bool drawBg = false;

    drawBg |= calendarWnd().shouldRenderBackdrop();
    drawBg |= calendar().shouldRenderBackdrop();

    if (!drawBg) {
        return;
    }

    gdi().setSubEngineLayer(SEL_DOWN);
    
    calendarWnd().drawBackdrop();
    calendarWnd().onRenderBackdrop();
    calendar().drawBackdrop();
    calendar().onRenderBackdrop();

    gdi().setSubEngineLayer(SEL_UP);
    gdi().pushSubBackground();
}