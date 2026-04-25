#include "focusborder.h"
#include "cachedinifile.h"
#include "systemfilenames.h"
#include "twin.h"
#include "../logger.h"

cFocusBorder::cFocusBorder(akui::cWindow* parent) {
    _parent = parent;
    _init = false;
    _show = false;
    _color = 0x0;
    _thickness = 2;
    _currentFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
    _prevFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
    _nextFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
}

void cFocusBorder::init() {
    _init = true;

    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("focus border", "show", _show);
    _color = ini.GetInt("focus border", "color", _color) | BIT(15);
    _thickness = ini.GetInt("focus border", "thickness", _thickness);

    s32 duration = ini.GetInt("focus border", "duration", 0);
    _animation.setDuration(duration);
}

void cFocusBorder::update() {
    if (!_init || _parent == NULL) {
        return;
    }

    cRect focus = _parent->focusRectangle();
    if (_currentFocus == focus) {
        return;
    }

    if (_nextFocus == focus) {
        cPoint pos = twin().point(_prevFocus.position(), _nextFocus.position(), _animation.value());
        cSize size = twin().point(_prevFocus.size(), _nextFocus.size(), _animation.value());
        _currentFocus = cRect(pos, size, false);
        return;
    }

    if (_currentFocus.size().x == 0 || _currentFocus.size().y == 0) {
        _currentFocus = focus;
        return;
    }

    _prevFocus = _currentFocus;
    _nextFocus = focus;
    _animation.play();
}

void cFocusBorder::draw(GRAPHICS_ENGINE engine) {
    if (!_init || !_show) {
        return;
    }

    if (_currentFocus.size().x == 0 || _currentFocus.size().y == 0) {
        return;
    }

    gdi().setPenColor(_color, engine);
    gdi().frameRect(_currentFocus.position().x, _currentFocus.position().y, _currentFocus.size().x, _currentFocus.size().y, _thickness, engine);
}

bool cFocusBorder::busy() const {
    return _animation.isPlaying();
}