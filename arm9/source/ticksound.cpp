#include "ticksound.h"
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "systemdetails.h"
#include "datetime.h"
#include "globalsettings.h"
#include "vfxmanager.h"
#include "tick_sound_bin.h"

#define TICK_VOLUME 64
#define TICK_VOLUME_MAX 127

cTickSound::cTickSound() {
    _running = false;
    _checkpoint = 0;
}

void cTickSound::play() {
    if (!_running) {
        return;
    }

    if (!sd().fifoStatus()) {
        return;
    }

    if (!gs().clockSound) {
        return;
    }

    u64 now = datetime().secondsInDay();
    if ((now - _checkpoint) <= 0 && now >= _checkpoint) {
        return;
    }

    if ((now - _checkpoint) > 2) {
        _checkpoint = now;
        return;
    }

    _checkpoint = now;

    u8 volume = datetime().seconds() == 0 ? TICK_VOLUME_MAX : TICK_VOLUME;

    vfxManager().playEffect(VFX_EFFECT::TICK, volume);
}

void cTickSound::enable() {
    _running = true;
}

void cTickSound::disable() {
    _running = false;
}