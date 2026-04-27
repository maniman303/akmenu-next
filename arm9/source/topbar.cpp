#include "topbar.h"
#include "cachedinifile.h"
#include "logger.h"
#include "personaldata.h"
#include "twin.h"
#include "systemfilenames.h"

cTopBar::cTopBar() : cWindow(NULL, "topbar") {
    _firstDraw = false;

    _animation.setDuration(16);

    _batteryMeter.setParent(this);
    _bootIcon.setParent(this);
    _fpsCounter.setParent(this);
    _smallClock.setParent(this);
    _smallDate.setParent(this);
    _userWnd.setParent(this);

    setEngine(GE_SUB);
}

cTopBar::~cTopBar() { }

void cTopBar::init() {
    // load bg, overlay, set size and position
    _background = createBMP15FromFile(SFN_TOPBAR_BG);
    _overlay = createBMP15FromFile(SFN_TOPBAR_OVERLAY);

    setSize(cSize(_background.width(), _background.height()));

    _batteryMeter.init();
    _bootIcon.init();
    _fpsCounter.init();
    _smallClock.init();
    _smallDate.init();

    if (!_background.valid()) {
        return;
    }

    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    bool colorize = ini.GetInt("topbar", "colorize", 0);
    if (colorize) {
        _background.colorize(personalData().color());
    }
}

void cTopBar::update() {
    if (size().y == 0) {
        return;
    }

    if (!_animation.isPlaying() && _firstDraw && gdi().getScreenTransparency(selectedEngine()) == 100) {
        _animation.play();
    }

    s16 posY = twin().int16(-size().y, 0, _animation.value());
    setRelativePosition(cPoint(0, posY));
}

void cTopBar::blink() {
    _batteryMeter.flipIcon();
    _smallClock.flipColon();
}

void cTopBar::draw() {
    _firstDraw = true;

    if (_background.valid()) {
        gdi().bitBlt(_background.buffer(), position().x, position().y, _background.width(), _background.height(), selectedEngine());
    }

    if (_overlay.valid()) {
        gdi().maskBlt(_overlay.buffer(), position().x, position().y, _overlay.width(), _overlay.height(), selectedEngine());
    }

    _batteryMeter.draw();
    _bootIcon.draw();
    _fpsCounter.draw();
    _smallClock.draw();
    _smallDate.draw();
    _userWnd.draw();
}