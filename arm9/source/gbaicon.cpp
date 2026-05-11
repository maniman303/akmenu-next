#include <string>
#include "gbaicon.h"
#include "cachedinifile.h"
#include "personaldata.h"
#include "systemfilenames.h"

cGbaIcon::cGbaIcon() {
    _bottom = false;
    _show = false;
    setEngine(GE_SUB);
}

void cGbaIcon::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("gba icon", "show", _show);
    int dx = ini.GetInt("gba icon", "x", 0);
    int dy = ini.GetInt("gba icon", "y", 0);
    setRelativePosition(cPoint(dx, dy));
}

void cGbaIcon::draw() {
    if (!_show) {
        return;
    }

    std::string filename = personalData().gbaBottomScreen() ? SFN_UI_GBA_BOTTOM_ICON : SFN_UI_GBA_TOP_ICON;
    if (!_icon.valid() || _icon.filename() != filename) {
        _icon = createBMP15FromFile(filename);
    }

    if (!_icon.valid()) {
        return;
    }

    gdi().maskBlt(_icon.buffer(), position().x, position().y, _icon.width(), _icon.height(), selectedEngine());
}