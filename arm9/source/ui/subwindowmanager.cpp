#include "subwindowmanager.h"
#include "analogclock.h"
#include "bigclock.h"
#include "calendar.h"
#include "calendarwnd.h"
#include "batterymeter.h"
#include "booticon.h"
#include "smalldate.h"
#include "smallclock.h"
#include "diskicon.h"
#include "userwnd.h"
#include "fpscounter.h"

cSubWindowManager::cSubWindowManager() {}

cSubWindowManager::~cSubWindowManager() {}

void cSubWindowManager::blink() {
    bigClock().blinkColon();
    smallClock().flipColon();
    batteryMeter().flipIcon();
}

void cSubWindowManager::update() {
    updateBackground();

    analogClock().draw();
    bigClock().draw();
    batteryMeter().draw();
    bootIcon().draw();
    smallDate().draw();
    smallClock().draw();
    userWindow().draw();
    fpsCounter().draw();
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