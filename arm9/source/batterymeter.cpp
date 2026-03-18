#include <nds.h>
#include <nds/system.h>
#include <math.h>
#include <chrono>
#include "batterymeter.h"
#include "globalsettings.h"
#include "inifile.h"
#include "systemfilenames.h"
#include "systemdetails.h"

cBatteryMeter::cBatteryMeter() : cWindow(NULL, "BatteryMeter") {
    _dx = 0;
    _dy = 0;
    _show = false;
    _start = 0;

    _size = cSize(1, 1);
    _position = cPoint(0, 0);
    _engine = GE_SUB;
}

cBatteryMeter::~cBatteryMeter() { }

static s64 getMillisecondsSinceEpoch() {
    auto tp = std::chrono::steady_clock::now();

    s64 ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        tp.time_since_epoch()
    ).count();

    return ms;
}

void cBatteryMeter::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _dx = ini.GetInt("battery icon", "x", 0);
    _dy = ini.GetInt("battery icon", "y", 0);
    _show = ini.GetInt("battery icon", "show", _show);
    _start = getMillisecondsSinceEpoch();
}

std::string cBatteryMeter::getBatteryFileName(int level) {
    std::string base = SFN_UI_DIRECTORY + gs().uiName + "/icons";
    
    if (level & BIT(0)) {
        return base + "/battery1.bmp";
    }

    return base + "/battery4.bmp";
}

void cBatteryMeter::draw() {
    if (!_battery.valid()) {
        _battery = createBMP15FromFile(getBatteryFileName(sd().batteryStatus()));
        gdi().maskBlt(_battery.buffer(), _dx, _dy, _battery.width(), _battery.height(), selectedEngine());
        return;
    }

    s64 now = getMillisecondsSinceEpoch();
    if (now - _start < 500) {
        gdi().maskBlt(_battery.buffer(), _dx, _dy, _battery.width(), _battery.height(), selectedEngine());
        return;
    }

    _start = now;
    _battery = createBMP15FromFile(getBatteryFileName(sd().batteryStatus()));
    gdi().maskBlt(_battery.buffer(), _dx, _dy, _battery.width(), _battery.height(), selectedEngine());
    return;
}