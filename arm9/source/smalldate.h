#pragma once

#include "bmp15.h"
#include "datetime.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

class cSmallDate : public cWindow {
  public:
    cSmallDate();

    ~cSmallDate() {}

  public:
    void init();

    void draw();

  protected:
    u16 _textColor;
    bool _show;
};

typedef t_singleton<cSmallDate> SmallDate_s;
inline cSmallDate& smallDate() {
    return SmallDate_s::instance();
}