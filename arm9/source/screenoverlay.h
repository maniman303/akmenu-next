#pragma once

#include "bmp15.h"
#include "window.h"

class cScreenOverlay : public cWindow {
  public:
    cScreenOverlay();

  public:
    void draw() override;

    cWindow& loadAppearance(const std::string& aFileName) override;

  protected:
    cBMP15 _background;
};

typedef t_singleton<cScreenOverlay> ScreenOverlay_s;
inline cScreenOverlay& screenOverlay() {
    return ScreenOverlay_s::instance();
}