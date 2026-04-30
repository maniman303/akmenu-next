#pragma once

#include "bmp15.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

#define PM_BATTERY_LOW   BIT(0)
#define PM_AC_PRESENT    BIT(1)

class cBootIcon : public cWindow {
  public:
    cBootIcon();

    ~cBootIcon();

  public:
    void init();
    void draw() override;

  protected:
    bool _show;
    cBMP15 _icon;
};

typedef t_singleton<cBootIcon> BootIcon_s;
inline cBootIcon& bootIcon() {
    return BootIcon_s::instance();
}