#include "rombootwnd.h"
#include "language.h"
#include "twin.h"
#include "logger.h"
#include "vfxmanager.h"
#include "../romlauncher.h"
#include "font/fontfactory.h"
#include "ui/windowmanager.h"
#include "tasks/screenfade.h"

cRomBootWnd::cRomBootWnd(std::string romPath, std::function<void()> onExit) :
 cWindow(NULL, "rom boot"),
 _launchText(this),
 _nameText(this),
 _pressText(this) {
    _romPath = romPath;
    _onExit = onExit;
    _romInfo.mayBeDSRom(romPath);

    _timer = 0;
    _pressAnimation.setDuration(30);

    _engine = GE_MAIN;
    _canRenderBackdrop = true;
    _scheduleBackdrop = true;

    setRelativePosition(cPoint(0, 0));
    setSize(cSize(SCREEN_WIDTH, SCREEN_HEIGHT));
}

cRomBootWnd::~cRomBootWnd() { }

cWindow& cRomBootWnd::loadAppearance(const std::string& aFileName) {
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
    if (_timer < 24) {
        gdi().setMainLayerTransparency(0, MEL_UP);
        _timer++;
        return;
    }

    if (!_pressAnimation.isPlaying()) {
        _pressAnimation.play();
    }
    
    if (_pressAnimation.isCompleted()) {
        if (_pressAnimation.isReversed()) {
            _pressAnimation.play();
        } else {
            _pressAnimation.reverse();
        }
    }

    u16 transparency = (u16)twin().int32(0, 100, _pressAnimation.value(), TWIN_EASE::EASE_OUT);
    // logger().info("Set transparency to: " + std::to_string(transparency));

    // TODO: Animate "Press START..." fade in and out
    gdi().setMainLayerTransparency(transparency, MEL_UP);
}

void cRomBootWnd::onFocused() {
    gdi().setScreenTransparency(0, GE_SUB);
    if (!_romInfo.isDSRom()) {
        windowManager().removeWindow(this);
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
    _pressText.setRelativePosition(cPoint(0, 168));
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
        gdi().setMainLayerTransparency(100, MEL_UP);

        if (_onExit) {
            _onExit();
        }
        windowManager().removeWindow(this);
    });
    
    task->schedule();
}

void cRomBootWnd::startRom() {
    gdi().setScreenTransparency(0, selectedEngine());
    disableInput();
    vfxManager().playEffect(VFX_EFFECT::SELECT);
    WorkIndicatorTask* task = new WorkIndicatorTask({&vfxManager()}, this, [this](){
        if (launchRom(_romPath, _romInfo, false, "") != ELaunchRomOk) {
            moveToMain();
        }
    });
    task->schedule();
}