#include <algorithm>
#include "animation.h"
#include "../timer.h"

Animation::Animation(u16 duration) {
    _duration = duration;
    _startTick = 0;
    _isPlaying = false;
    _isReversed = false;
}

void Animation::play() {
    _isReversed = false;
    start();
}

void Animation::reverse() {
    _isReversed = true;
    start();
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

bool Animation::isCompleted() {
    return _isPlaying && _startTick + _duration <= timer().getTick();
}

bool Animation::isPlaying() {
    return _isPlaying && _startTick + _duration > timer().getTick();
}

bool Animation::isReversed() {
    return _isReversed;
}

u16 Animation::duration() {
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
        double incPerTick = 100.0 / static_cast<double>(_duration);
        res = std::min((u16)100, static_cast<u16>(incPerTick * static_cast<double>(ticksSincStart)));
    }

    return _isReversed ? 100 - res : res;
}