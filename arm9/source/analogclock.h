#pragma once

#include "point.h"
#include "singleton.h"
#include "window.h"

using namespace akui;

#define PM_BATTERY_LOW   BIT(0)
#define PM_AC_PRESENT    BIT(1)

class cAnalogClock : public cWindow {
  public:
    cAnalogClock();

    ~cAnalogClock();

  public:
    void init();

    void draw() override;

    cWindow& loadAppearance(const std::string& aFileName) override { return *this; }

  protected:
    bool _show;
    u16 _lengthSeconds;
    u16 _lengthMinutes;
    u16 _lengthHours;
    u16 _colorSeconds;
    u16 _colorMinutes;
    u16 _colorHours;
    u16 _colorDot;
};

typedef t_singleton<cAnalogClock> AnalogClock_s;
inline cAnalogClock& analogClock() {
    return AnalogClock_s::instance();
}