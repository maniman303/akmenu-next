#include "focusborder.h"
#include "cachedinifile.h"
#include "systemfilenames.h"
#include "twin.h"
#include "../logger.h"
#include "../personaldata.h"

cFocusBorder::cFocusBorder(cWindow* parent) {
    _parent = parent;
    _init = false;
    _show = false;
    _color = 0x0;
    _thickness = 2;
    _clickOffset = 0;
    _currentFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
    _prevFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
    _nextFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
}

void cFocusBorder::init() {
    _init = true;

    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("focus border", "show", _show);
    _color = ini.GetColor("focus border", "color", _color);
    _thickness = ini.GetInt("focus border", "thickness", _thickness);

    s32 duration = ini.GetInt("focus border", "duration", 0);
    _moveController.setDuration(duration);
    _clickController.setDuration(6);

    if (!_show) {
        return;
    }

    _ftr = createBMP15FromFile(SFN_UI_FOCUS_TR);
    _ftl = createBMP15FromFile(SFN_UI_FOCUS_TL);
    _fbr = createBMP15FromFile(SFN_UI_FOCUS_BR);
    _fbl = createBMP15FromFile(SFN_UI_FOCUS_BL);

    bool colorize = ini.GetInt("focus border", "colorize", 0);
    if (colorize) {
        if (_color == 0x8000) {
            _color = personalData().color();
        }

        _ftr.colorize(personalData().color());
        _ftl.colorize(personalData().color());
        _fbr.colorize(personalData().color());
        _fbl.colorize(personalData().color());
    }
}

void cFocusBorder::reset() {
    _currentFocus = cRect(cPoint(0, 0), cPoint(0, 0), false);
}

void cFocusBorder::click() {
    if (_clickController.isPlaying() && !_clickController.isCompleted()) {
        return;
    }

    _clickController.play();
}

void cFocusBorder::update() {
    if (!_init || _parent == NULL) {
        return;
    }

    if (_clickController.isCompleted() && !_clickController.isReversed()) {
        _clickController.reverse();
    }

    _clickOffset = twin().int16(0, 2, _clickController.value(), TWIN_EASE::EASE_OUT);

    cRect focus = _parent->focusRectangle();
    if (_currentFocus == focus) {
        return;
    }

    if (_nextFocus == focus) {
        cPoint pos = twin().point(_prevFocus.position(), _nextFocus.position(), _moveController.value(), TWIN_EASE::EASE_OUT);
        cSize size = twin().point(_prevFocus.size(), _nextFocus.size(), _moveController.value(), TWIN_EASE::EASE_OUT);
        _currentFocus = cRect(pos, size, false);
        return;
    }

    if (_currentFocus.size().x == 0 || _currentFocus.size().y == 0) {
        _currentFocus = focus;
        return;
    }

    _prevFocus = _currentFocus;
    _nextFocus = focus;
    _moveController.play();
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
    if (_ftl.width() + _ftr.width() <= (u32)width && _ftl.height() + _fbl.height() <= (u32)height) {
        if (_ftl.valid()) {
            drawRect = false;
            gdi().maskBlt(_ftl.buffer(), startX + _clickOffset, startY + _clickOffset, _ftl.width(), _ftl.height(), engine);
        }

        if (_ftr.valid()) {
            drawRect = false;
            gdi().maskBlt(_ftr.buffer(), startX + width - _ftr.width() - _clickOffset, startY + _clickOffset, _ftr.width(), _ftr.height(), engine);
        }

        if (_fbl.valid()) {
            drawRect = false;
            gdi().maskBlt(_fbl.buffer(), startX + _clickOffset, startY + height - _fbl.height() - _clickOffset, _fbl.width(), _fbl.height(), engine);
        }

        if (_fbr.valid()) {
            drawRect = false;
            gdi().maskBlt(_fbr.buffer(), startX + width - _fbr.width() - _clickOffset, startY + height - _fbr.height() - _clickOffset, _fbr.width(), _fbr.height(), engine);
        }
    }

    if (drawRect) {
        gdi().setPenColor(_color, engine);
        gdi().frameRect(startX, startY, width, height, _thickness + _clickOffset, engine);
        return;
    }
}

bool cFocusBorder::busy() const {
    return (_moveController.isPlaying() && !_moveController.isCompleted()) || (_clickController.isPlaying() && !_clickController.isCompleted());
}