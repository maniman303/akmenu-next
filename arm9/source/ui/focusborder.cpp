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

    if (!_show) {
        return;
    }

    _ftr = createBMP15FromFile(SFN_UI_FOCUS_TR);
    _ftl = createBMP15FromFile(SFN_UI_FOCUS_TL);
    _fbr = createBMP15FromFile(SFN_UI_FOCUS_BR);
    _fbl = createBMP15FromFile(SFN_UI_FOCUS_BL);
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
        cPoint pos = twin().point(_prevFocus.position(), _nextFocus.position(), _animation.value(), TWIN_EASE::EASE_OUT);
        cSize size = twin().point(_prevFocus.size(), _nextFocus.size(), _animation.value(), TWIN_EASE::EASE_OUT);
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

    bool drawRect = true;
    s32 startX = _currentFocus.position().x;
    s32 startY = _currentFocus.position().y;
    s32 width = _currentFocus.size().x;
    s32 height = _currentFocus.size().y;
    if (_ftl.valid()) {
        drawRect = false;
        gdi().maskBlt(_ftl.buffer(), startX, startY, _ftl.width(), _ftl.height(), engine);
    }

    if (_ftr.valid()) {
        drawRect = false;
        gdi().maskBlt(_ftr.buffer(), startX + width - _ftr.width(), startY, _ftr.width(), _ftr.height(), engine);
    }

    if (_fbl.valid()) {
        drawRect = false;
        gdi().maskBlt(_fbl.buffer(), startX, startY + height - _fbl.height(), _fbl.width(), _fbl.height(), engine);
    }

    if (_fbr.valid()) {
        drawRect = false;
        gdi().maskBlt(_fbr.buffer(), startX + width - _fbr.width(), startY + height - _fbr.height(), _fbr.width(), _fbr.height(), engine);
    }

    if (drawRect) {
        gdi().setPenColor(_color, engine);
        gdi().frameRect(startX, startY, width, height, _thickness, engine);
        return;
    }
}

bool cFocusBorder::busy() const {
    return _animation.isPlaying();
}