#include "keymessage.h"

cKeyMessage::cKeyMessage(u32 keysHeld, u32 keysUp, u32 keysDown, u32 keysDownRepeat) {
    _keysHeld = keysHeld;
    _keysUp = keysUp;
    _keysDown = keysDown;
    _keysDownRepeat = keysDownRepeat;
}

bool cKeyMessage::isKeyHeld(KEYPAD_BITS key) { return _keysHeld & key; }
bool cKeyMessage::isKeyUp(KEYPAD_BITS key) { return _keysUp & key; }
bool cKeyMessage::isKeyDown(KEYPAD_BITS key) { return _keysDown & key; }
bool cKeyMessage::isKeyShift(KEYPAD_BITS key) { return (_keysHeld & key) || (_keysDown & key); }