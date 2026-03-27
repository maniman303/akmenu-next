#include <string>
#include "booticon.h"
#include "globalsettings.h"
#include "inifile.h"
#include "systemfilenames.h"

cBootIcon::cBootIcon() {
    setEngine(GE_SUB);
    _show = false;
}

cBootIcon::~cBootIcon() {}

void cBootIcon::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _show = ini.GetInt("boot icon", "show", _show);
    int dx = ini.GetInt("boot icon", "x", 0);
    int dy = ini.GetInt("boot icon", "y", 0);
    setRelativePosition(cPoint(dx, dy));
}

void cBootIcon::draw() {
    if (!_show) {
        return;
    }

    std::string filename = gs().autorunWithLastRom ? SFN_UI_BOOT_AUTO_ICON : SFN_UI_BOOT_MANUAL_ICON;
    if (!_icon.valid() || _icon.filename() != filename) {
        destroyBMP15(_icon);
        _icon = createBMP15FromFile(filename);
    }

    if (!_icon.valid()) {
        return;
    }

    gdi().maskBlt(_icon.buffer(), position().x, position().y, _icon.width(), _icon.height(), selectedEngine());
}