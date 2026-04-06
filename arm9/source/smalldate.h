#pragma once

#include "singleton.h"
#include "bmp15.h"
#include "datetime.h"
#include "ui/point.h"
#include "ui/window.h"

using namespace akui;

class cSmallDate : public akui::cWindow {
  public:
    cSmallDate();

    ~cSmallDate() {}

  public:
    void init();

    void draw();

    akui::cWindow& loadAppearance(const std::string& aFileName) { return *this; }

  protected:
    u16 _textColor;
    bool _show;
};

typedef t_singleton<cSmallDate> SmallDate_s;
inline cSmallDate& smallDate() {
    return SmallDate_s::instance();
}