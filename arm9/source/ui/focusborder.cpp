#include "focusborder.h"
#include "cachedinifile.h"
#include "systemfilenames.h"
#include "../logger.h"

cFocusBorder::cFocusBorder(akui::cWindow* parent) {
    _parent = parent;
    _init = false;
    _show = false;
    _color = 0x0;
    _thickness = 2;
    _currentFocus = cRect(cPoint(0, 0), cSize(0, 0), false);
}

void cFocusBorder::init() {
    _init = true;

    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("focus border", "show", _show);
    _color = ini.GetInt("focus border", "color", _color) | BIT(15);
    _thickness = ini.GetInt("focus border", "thickness", _thickness);

    // logger().info("Init focus border.");
}

void cFocusBorder::update() {
    if (!_init || _parent == NULL) {
        return;
    }

    // logger().info("Updating focus border.");

    _currentFocus = _parent->focusRectangle();
}

void cFocusBorder::draw(GRAPHICS_ENGINE engine) {
    if (!_init || !_show) {
        // logger().info("Skipping focus border due to init / show.");
        return;
    }

    if (_currentFocus.size().x == 0 || _currentFocus.size().y == 0) {
        // logger().info("Skipping focus border due to size.");
        return;
    }

    // logger().info("Drawing focus border.");

    gdi().setPenColor(_color, engine);
    gdi().frameRect(_currentFocus.position().x, _currentFocus.position().y, _currentFocus.size().x, _currentFocus.size().y, _thickness, engine);
}

bool cFocusBorder::busy() const {
    return false;
}