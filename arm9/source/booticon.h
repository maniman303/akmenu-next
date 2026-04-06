#pragma once

#include "bmp15.h"
#include "ui/point.h"
#include "ui/bitmapdesc.h"
#include "ui/window.h"
#include "singleton.h"

using namespace akui;

#define PM_BATTERY_LOW   BIT(0)
#define PM_AC_PRESENT    BIT(1)

class cBootIcon : public cWindow {
  public:
    cBootIcon();

    ~cBootIcon();

  public:
    void init();

    void draw() override;

    cWindow& loadAppearance(const std::string& aFileName) override { return *this; }

  protected:
    bool _show;
    cBMP15 _icon;
};

typedef t_singleton<cBootIcon> BootIcon_s;
inline cBootIcon& bootIcon() {
    return BootIcon_s::instance();
}