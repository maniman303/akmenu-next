#include "touchmessage.h"

cTouchMessage::cTouchMessage(u16 x, u16 y, s16 mx, s16 my, bool touchDown, bool touchUp, bool touchMove) {
    _position = cPoint(x, y);
    _movement = cPoint(mx, my);
    _touchDown = touchDown;
    _touchUp = touchUp;
    _touchMove = touchMove;
}

cPoint cTouchMessage::position() { return _position; }
cPoint cTouchMessage::movement() { return _movement; }
bool cTouchMessage::down() { return _touchDown; }
bool cTouchMessage::up() { return _touchUp; }
bool cTouchMessage::move() { return _touchMove; }