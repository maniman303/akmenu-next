#pragma once

#include <nds.h>
#include <string>
#include "singleton.h"

class cTickSound {
  public:
    cTickSound();

    ~cTickSound();

    bool load(std::string filepath);

    void play();

  protected:
    void unload();

    u8* _rawData;
    u32  _dataSize;
    u8* _pcmStart;
    u32  _sampleRate;
    int  _soundFormat;
    u64 _checkpoint;
    int _soundId;
    int _scheduleSoundId;
    int _schedule;
};

typedef t_singleton<cTickSound> TickSound_s;
inline cTickSound& tickSound() {
    return TickSound_s::instance();
}