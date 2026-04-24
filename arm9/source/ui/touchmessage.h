#pragma once

#include <nds.h>
#include "point.h"

class cTouchMessage {
  public:
    cTouchMessage(u16 x, u16 y, s16 mx, s16 my, bool touchDown, bool touchUp, bool touchMove);

    cPoint position();
    cPoint movement();
    bool down();
    bool up();
    bool move();

  private:
    cPoint _position;
    cPoint _movement;
    bool _touchDown;
    bool _touchUp;
    bool _touchMove;
};
