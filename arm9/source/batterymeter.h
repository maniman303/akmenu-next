#pragma once

#include "bmp15.h"
#include "point.h"
#include "singleton.h"
#include "bitmapdesc.h"
#include "window.h"

using namespace akui;

#define PM_BATTERY_LOW   BIT(0)
#define PM_AC_PRESENT    BIT(1)

class cBatteryMeter : public akui::cWindow {
  public:
    cBatteryMeter();

    ~cBatteryMeter();

  public:
    void init();

    void draw();

    akui::cWindow& loadAppearance(const std::string& aFileName) { return *this; }

  protected:
    std::string getBatteryFileName(int level);

    int _dx;
    int _dy;
    bool _show;
    s64 _start;
    cBMP15 _battery;
};

typedef t_singleton<cBatteryMeter> BatteryMeter_s;
inline cBatteryMeter& batteryMeter() {
    return BatteryMeter_s::instance();
}