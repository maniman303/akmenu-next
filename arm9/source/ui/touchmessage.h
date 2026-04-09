#pragma once

#include <nds.h>
#include "point.h"

class cTouchMessage {
  public:
    cTouchMessage(u16 x, u16 y, bool touchDown, bool touchUp, bool touchMove);

    akui::cPoint position();
    bool down();
    bool up();
    bool move();

  private:
    akui::cPoint _position;
    bool _touchDown;
    bool _touchUp;
    bool _touchMove;
};
