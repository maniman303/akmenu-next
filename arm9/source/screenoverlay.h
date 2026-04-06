#pragma once

#include "bmp15.h"
#include "ui/window.h"

class cScreenOverlay : public akui::cWindow {
  public:
    cScreenOverlay();

  public:
    void draw() override;

    akui::cWindow& loadAppearance(const std::string& aFileName) override { return *this; }

  protected:
    cBMP15 _background;
};

typedef t_singleton<cScreenOverlay> ScreenOverlay_s;
inline cScreenOverlay& screenOverlay() {
    return ScreenOverlay_s::instance();
}