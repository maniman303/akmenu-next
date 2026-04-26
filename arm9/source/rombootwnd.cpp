#include "rombootwnd.h"
#include "language.h"
#include "../romlauncher.h"
#include "font/fontfactory.h"
#include "ui/windowmanager.h"
#include "tasks/screenfade.h"

cRomBootWnd::cRomBootWnd(std::string romPath, std::function<void()> onExit) :
 akui::cWindow(NULL, "rom boot"),
 _launchText(this),
 _nameText(this),
 _pressText(this) {
    _romPath = romPath;
    _onExit = onExit;
    _romInfo.mayBeDSRom(romPath);

    _engine = GE_MAIN;
    _canRenderBackdrop = true;
    _scheduleBackdrop = true;

    setRelativePosition(cPoint(0, 0));
    setSize(cSize(SCREEN_WIDTH, SCREEN_HEIGHT));
}

cRomBootWnd::~cRomBootWnd() { }

akui::cWindow& cRomBootWnd::loadAppearance(const std::string& aFileName) {
    return *this;
}

bool cRomBootWnd::processKeyMessage(cKeyMessage message) {
    if (message.isKeyDown(KEY_START) || message.isKeyDown(KEY_A)) {
        disableInput();
        startRom();

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
        startRom();

        return true;
    }

    return false;
}

void cRomBootWnd::update() {
    // TODO: Animate "Press START..." fade in and out
}

void cRomBootWnd::onGainedFocus() {
    gdi().setScreenTransparency(0, GE_SUB);
    if (!_romInfo.isDSRom()) {
        akui::windowManager().removeWindow(this);
        if (_onExit) {
            _onExit();
        }
        return;
    }

    _launchText.setText(LANG("rom boot", "launching"));
    _launchText.setRelativePosition(cPoint(0, 56));
    _launchText.setSize(cSize(SCREEN_WIDTH, 32));
    _launchText.setTextColor(0x0 | BIT(15));
    _launchText.setCentered(true);
    _launchText.setFont(false);

    _pressText.setText(LANG("rom boot", "press"));
    _pressText.setRelativePosition(cPoint(0, 172));
    _pressText.setSize(cSize(SCREEN_WIDTH, 32));
    _pressText.setTextColor(0x0 | BIT(15));
    _pressText.setCentered(true);

    std::string romName = _romInfo.getDsLocTitle();
    u16 romNameHeight = font().TextHeight(romName);
    u16 romNameY = (196 - romNameHeight) >> 1;
    _nameText.setText(romName);
    _nameText.setRelativePosition(cPoint(112, romNameY));
    _nameText.setSize(cSize(128, romNameHeight + font().GetDescend()));
    _nameText.setTextColor(0x0 | BIT(15));
}

void cRomBootWnd::draw() {
    _pressText.draw();
}

void cRomBootWnd::drawBackdrop() {
    if (!_romInfo.isDSRom()) {
        return;
    }

    _launchText.draw();
    _romInfo.drawDSRomIcon(64, 82, false, selectedEngine());
    _nameText.draw();
}

void cRomBootWnd::moveToMain() {
    ScreenFadeTask* task = new ScreenFadeTask(true, false, false);
    task->setOnCompleted([this](){
        // TODO: Set UP layer transparency to 100

        if (_onExit) {
            _onExit();
        }
        akui::windowManager().removeWindow(this);
    });
    
    task->schedule();
}

void cRomBootWnd::startRom() {
    gdi().setScreenTransparency(0, selectedEngine());
    if (launchRom(_romPath, _romInfo, false, "") != ELaunchRomOk) {
        moveToMain();
    }
}