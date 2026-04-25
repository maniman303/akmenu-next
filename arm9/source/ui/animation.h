#pragma once

#include <nds.h>

class Animation {
  public:
    Animation(u16 duration);

    void play();
    void reverse();
    void reset();
    bool isCompleted();
    bool isPlaying();
    bool isReversed();
    u16 duration();
    u16 value();

  private:
    inline void start();

    u16 _duration;
    u32 _startTick;
    bool _isPlaying;
    bool _isReversed;
};