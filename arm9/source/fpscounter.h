#pragma once

#include "point.h"
#include "singleton.h"
#include "window.h"

class cFpsCounter : public cWindow {
  public:
    cFpsCounter();

    ~cFpsCounter() override {}

  public:
    void init();

    void draw() override;

    cWindow& loadAppearance(const std::string& aFileName) { return *this; }

  protected:
    u16 _textColor;
    bool _show;
};

typedef t_singleton<cFpsCounter> FpsCounter_s;
inline cFpsCounter& fpsCounter() {
    return FpsCounter_s::instance();
}