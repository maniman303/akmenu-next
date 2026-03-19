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
    double _checkpoint;
    bool _isFresh;
    int _soundId;
};

typedef t_singleton<cTickSound> TickSound_s;
inline cTickSound& tickSound() {
    return TickSound_s::instance();
}