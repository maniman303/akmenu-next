#include "diskicon.h"
#include "globalsettings.h"
#include "cachedinifile.h"
#include "systemfilenames.h"

using namespace akui;

cDiskIcon::cDiskIcon() : cWindow(NULL, "diskicon") {
    _show = false;
    _blink = false;
    _engine = GE_MAIN;
    _isFocusable = false;
}

void cDiskIcon::draw() {
    if (!_show) {
        return;
    }

    if (_blink) {
        return;
    }

    gdi().maskBlt(_icon.buffer(), position().x, position().y, _icon.width(), _icon.height(), selectedEngine());
}

cWindow& cDiskIcon::loadAppearance(const std::string& aFileName) {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("disk icon", "show", _show);
    if (!_show) {
        return *this;
    }

    u16 x = ini.GetInt("disk icon", "x", 238);
    u16 y = ini.GetInt("disk icon", "y", 172);
    setRelativePosition(cPoint(x, y));

    _icon = createBMP15FromFile(aFileName);

    _show = _icon.valid();

    return *this;
}

void cDiskIcon::blink(void) {
    _blink = !_blink;
}

void cDiskIcon::turnOn() {
    _blink = true;
}

void cDiskIcon::turnOff() {
    _blink = false;
}
