#pragma once

#include "singleton.h"
#include "batterymeter.h"
#include "booticon.h"
#include "fpscounter.h"
#include "smallclock.h"
#include "smalldate.h"
#include "userwnd.h"
#include "bmp15.h"
#include "ui/animation.h"
#include "ui/point.h"
#include "ui/window.h"

class cTopBar : public cWindow {
  public:
    cTopBar();

    ~cTopBar() override;

  public:
    void init();
    void blink();
    void update() override;
    void draw() override;
    cWindow& loadAppearance(const std::string& aFileName) override { return *this; }

  private:
    bool _firstDraw;
    Animation _animation;
    cBMP15 _background;
    cBMP15 _overlay;
    cBatteryMeter _batteryMeter;
    cBootIcon _bootIcon;
    cFpsCounter _fpsCounter;
    cSmallClock _smallClock;
    cSmallDate _smallDate;
    cUserWindow _userWnd;
};

typedef t_singleton<cTopBar> TopBar_s;
inline cTopBar& topbar() {
    return TopBar_s::instance();
}