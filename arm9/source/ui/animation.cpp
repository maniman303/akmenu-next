#include <algorithm>
#include "animation.h"
#include "../divider.h"
#include "../timer.h"

Animation::Animation() : Animation(0) {}

Animation::Animation(s32 duration) {
    _startTick = 0;
    _isPlaying = false;
    _isReversed = false;
    setDuration(duration);
}

void Animation::play() {
    _isReversed = false;
    start();
}

void Animation::reverse() {
    _isReversed = true;
    start();
}

void Animation::complete() {
    _isPlaying = true;
    _startTick = timer().getTick() - _duration;
}

void Animation::start() {
    _isPlaying = true;
    _startTick = timer().getTick();
}

void Animation::reset() {
    _isPlaying = false;
    _isReversed = false;
    _startTick = 0;
}

bool Animation::isCompleted() const {
    return _isPlaying && _startTick + _duration <= timer().getTick();
}

bool Animation::isPlaying() const {
    return _isPlaying;
}

bool Animation::isReversed() const {
    return _isReversed;
}

void Animation::setDuration(s32 duration) {
    if (duration < 0) {
        duration = 0;
    }

    _duration = (u16)duration;
}

u16 Animation::duration() const {
    return _duration;
}

u16 Animation::value() {
    if (!_isPlaying) {
        return 0;
    }

    u16 res = 0;
    u32 ticksSincStart = timer().getTick() - _startTick;
    if (ticksSincStart >= _duration) {
        res = 100;
    } else if (ticksSincStart > 0) {
        s32 incRestPerTick = 0;
        s32 incPerTick = hw::divide(100, _duration, incRestPerTick);
        res = std::min((u16)100, (u16)(ticksSincStart * incPerTick + hw::divide(ticksSincStart * incRestPerTick, _duration)));
    }

    return _isReversed ? 100 - res : res;
}