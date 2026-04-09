#pragma once

#include <nds.h>

class cKeyMessage {
  public:
    cKeyMessage(u32 keysHeld, u32 keysUp, u32 keysDown, u32 keysDownRepeat);

    bool isKeyHeld(KEYPAD_BITS key);
    bool isKeyUp(KEYPAD_BITS key);
    bool isKeyDown(KEYPAD_BITS key);
    bool isKeyShift(KEYPAD_BITS key);

  private:
    u32 _keysHeld;
    u32 _keysUp;
    u32 _keysDown;
    u32 _keysDownRepeat;
};
