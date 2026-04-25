#pragma once

#include <nds.h>
#include "window.h"
#include "animation.h"
#include "../workindicator.h"
#include "../gdi.h"

class cFocusBorder : public WorkIndicator {
  public:
    cFocusBorder(akui::cWindow* parent);

    void init();
    void update();
    void draw(GRAPHICS_ENGINE engine);
    bool busy() const override;

  private:
    akui::cWindow* _parent;
    bool _init;
    bool _show;
    u16 _color;
    u16 _thickness;
    cRect _currentFocus;
    cRect _prevFocus;
    cRect _nextFocus;
    Animation _animation;
};