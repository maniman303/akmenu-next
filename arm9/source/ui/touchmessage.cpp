#include "touchmessage.h"

cTouchMessage::cTouchMessage(u16 x, u16 y, bool touchDown, bool touchUp, bool touchMove) {
    _position = akui::cPoint(x, y);
    _touchDown = touchDown;
    _touchUp = touchUp;
    _touchMove = touchMove;
}

akui::cPoint cTouchMessage::position() { return _position; }
bool cTouchMessage::down() { return _touchDown; }
bool cTouchMessage::up() { return _touchUp; }
bool cTouchMessage::move() { return _touchMove; }