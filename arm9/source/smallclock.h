#pragma once

#include <chrono>
#include "singleton.h"
#include "bmp15.h"
#include "datetime.h"
#include "ui/point.h"
#include "ui/window.h"

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