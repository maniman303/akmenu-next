#include <nds.h>
#include <nds/system.h>
#include <math.h>
#include "batterymeter.h"
#include "irqs.h"
#include "fifotool.h"
#include "globalsettings.h"
#include "inifile.h"
#include "systemfilenames.h"
#include "systemdetails.h"
#include "datetime.h"

cBatteryMeter::cBatteryMeter() : cWindow(NULL, "BatteryMeter") {
    _dx = 0;
    _dy = 0;
    _show = false;
    _flip = false;

    _size = cSize(1, 1);
    _position = cPoint(0, 0);
    _engine = GE_SUB;
}

cBatteryMeter::~cBatteryMeter() { }

void cBatteryMeter::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _dx = ini.GetInt("battery icon", "x", 0);
    _dy = ini.GetInt("battery icon", "y", 0);
    _show = ini.GetInt("battery icon", "show", _show);
}

void cBatteryMeter::flipIcon() {
    _flip = !_flip;
}

int cBatteryMeter::getBatteryType() {
    if (!sd().fifoStatus()) {
        return -1;
    }

    int level = sd().batteryStatus();

    if (level & 0x2) {
        return 2;
    }

    return level & 0x1;
}

void cBatteryMeter::draw() {
    std::string newFile = SFN_UI_DIRECTORY + gs().uiName + "/icons";
    int type = getBatteryType();
    if (type < 0) {
        newFile += "/battery4.bmp";
    } else if (type == 1) {
        newFile += "/battery1.bmp";
    } else if (type == 2) {
        newFile += "/batterycharge.bmp";
    } else {
        newFile += "/battery4.bmp";
    }

    bool visible = type != 1 || _flip;
    if (!visible) {
        return;
    }

    _battery = createBMP15FromFile(newFile);

    gdi().maskBlt(_battery.buffer(), _dx, _dy, _battery.width(), _battery.height(), selectedEngine());
}