#pragma once

#include <nds.h>

class Animation {
  public:
    Animation();
    Animation(s32 duration);

    void play();
    void reverse();
    void reset();
    bool isCompleted() const;
    bool isPlaying() const;
    bool isReversed() const;
    void setDuration(s32 duration);
    u16 duration() const;
    u16 value();

  private:
    inline void start();

    u16 _duration;
    u32 _startTick;
    bool _isPlaying;
    bool _isReversed;
};