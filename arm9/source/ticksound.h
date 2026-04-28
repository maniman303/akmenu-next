#pragma once

#include <nds.h>
#include <string>
#include "singleton.h"

class cTickSound {
  public:
    cTickSound();

    void enable();
    void disable();
    void play();

  protected:
    bool _running;
    u64 _checkpoint;
};

typedef t_singleton<cTickSound> TickSound_s;
inline cTickSound& tickSound() {
    return TickSound_s::instance();
}
