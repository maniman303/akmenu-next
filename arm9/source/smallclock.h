#pragma once

#include <chrono>
#include "bmp15.h"
#include "datetime.h"
#include "point.h"
#include "singleton.h"
#include "window.h"

using namespace akui;

class cSmallClock : public akui::cWindow {
  public:
    cSmallClock();

    ~cSmallClock() {}

  public:
    void init();

    void draw();

    void flipColon();

    akui::cWindow& loadAppearance(const std::string& aFileName) { return *this; }

  protected:
    u16 _textColor;
    bool _show;
    bool _showColon;
};

typedef t_singleton<cSmallClock> SmallClock_s;
inline cSmallClock& smallClock() {
    return SmallClock_s::instance();
}