#include "rombootwnd.h"
#include "../romlauncher.h"
#include "ui/windowmanager.h"
#include "tasks/screenfade.h"

cRomBootWnd::cRomBootWnd(std::string romPath, std::function<void()> onExit) {
    _romPath = romPath;
    _onExit = onExit;
    _romInfo.mayBeDSRom(romPath);

    _canRenderBackdrop = true;
    _scheduleBackdrop = true;
}

akui::cWindow& cRomBootWnd::loadAppearance(const std::string& aFileName) {
    return *this;
}

bool cRomBootWnd::processKeyMessage(cKeyMessage message) {
    if (message.isKeyDown(KEY_START) || message.isKeyDown(KEY_A)) {
        disableInput();
        if (launchRom(_romPath, _romInfo, false, "") != ELaunchRomOk) {
            moveToMain();
        }

        return true;
    }

    if (message.isKeyDown(KEY_B)) {
        disableInput();
        moveToMain();

        return true;
    }

    return false;
}

bool cRomBootWnd::processTouchMessage(cTouchMessage message) {
    if (message.down() &&
        message.position().x > 0 && message.position().x < SCREEN_WIDTH -1 &&
        message.position().y > 0 && message.position().y < SCREEN_HEIGHT - 1) {
        disableInput();
        if (launchRom(_romPath, _romInfo, false, "") != ELaunchRomOk) {
            moveToMain();
        }

        return true;
    }

    return false;
}

void cRomBootWnd::update() {
    // TODO: Animate "Press START..." fade in and out
}

void cRomBootWnd::onGainedFocus() {
    gdi().setScreenTransparency(0, GE_SUB);
    if (_romInfo.isDSRom()) {
        return;
    }

    akui::windowManager().removeWindow(this);
    if (_onExit) {
        _onExit();
    }
}

void cRomBootWnd::draw() {
    // TODO: Implement
}

void cRomBootWnd::drawBackdrop() {
    // TODO: Implement
}

void cRomBootWnd::moveToMain() {
    ScreenFadeTask* task = new ScreenFadeTask(true, false, false);
    task->setOnCompleted([this](){
        // TODO: Set UP layer transparency to 100

        akui::windowManager().removeWindow(this);
        if (_onExit) {
            _onExit();
        }
    });
    
    task->schedule();
}