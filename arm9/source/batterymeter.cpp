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
    _checkpoint = 0;

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

std::string cBatteryMeter::getBatteryFileName() {
    std::string base = SFN_UI_DIRECTORY + gs().uiName + "/icons";
    
    if (!sd().fifoStatus()) {
        return base + "/battery1.bmp";
    }

    u64 now = datetime().secondsInDay();
    if ((now - _checkpoint) <= 0 && now >= _checkpoint) {
        return "";
    }

    fifoSendValue32(FIFO_USER_01, MENU_MSG_BATTERY_STATE);

    int iter = 0;
    while(!fifoCheckValue32(FIFO_USER_04) && iter < 20000) {
		iter++;
	}

    if (!fifoCheckValue32(FIFO_USER_04)) {
        return "";
    }

    _checkpoint = now;

    u32 level = fifoGetValue32(FIFO_USER_04);

    if (level <= 1) {
        return base + "/battery1.bmp";
    }

    return base + "/battery4.bmp";
}

void cBatteryMeter::draw() {
    std::string newFile = getBatteryFileName();
    if (newFile != "") {
        _battery = createBMP15FromFile(newFile);
    }

    gdi().maskBlt(_battery.buffer(), _dx, _dy, _battery.width(), _battery.height(), selectedEngine());
    return;
}