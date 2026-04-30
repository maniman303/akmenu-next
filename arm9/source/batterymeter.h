#pragma once

#include "bmp15.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

#define PM_BATTERY_LOW   BIT(0)
#define PM_AC_PRESENT    BIT(1)

class cBatteryMeter : public cWindow {
  public:
    cBatteryMeter();

    ~cBatteryMeter();

  public:
    void init();

    void flipIcon();

    void draw() override;

  protected:
    int getBatteryType();

    bool _show;
    bool _flip;
    cBMP15 _battery;
};

typedef t_singleton<cBatteryMeter> BatteryMeter_s;
inline cBatteryMeter& batteryMeter() {
    return BatteryMeter_s::instance();
}