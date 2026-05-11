#pragma once

#include "bmp15.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

class cGbaIcon : public cWindow {
  public:
    cGbaIcon();

  public:
    void init();
    void draw() override;

  protected:
    bool _bottom;
    bool _show;
    cBMP15 _icon;
};

typedef t_singleton<cGbaIcon> GbaIcon_s;
inline cGbaIcon& gbaIcon() {
    return GbaIcon_s::instance();
}