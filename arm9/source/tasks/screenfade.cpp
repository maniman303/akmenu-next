#include "screenfade.h"
#include "taskcruncher.h"
#include "../logger.h"

ScreenFadeTask::ScreenFadeTask() : ScreenFadeTask::ScreenFadeTask(true, true, true) {}

ScreenFadeTask::ScreenFadeTask(bool fadeIn) : ScreenFadeTask::ScreenFadeTask(true, true, fadeIn) {}

ScreenFadeTask::ScreenFadeTask(bool main, bool sub, bool fadeIn) {
    _main = main;
    _sub = sub;
    _fadeIn = fadeIn;
    _animation.setDuration(20);
}

s16 ScreenFadeTask::process(s16 iter) {
    logger().info("Processing fade with iter: " + std::to_string(iter));
    
    if (iter == 0 && !_animation.isPlaying()) {
        _animation.play();
    }

    u16 value = _fadeIn ? _animation.value() : 100 - _animation.value();
    
    if (_main) {
        gdi().setScreenTransparency(value, GE_MAIN);
    }

    if (_sub) {
        gdi().setScreenTransparency(value, GE_SUB);
    }

    if (iter != 0 && _animation.isCompleted()) {
        logger().info("Fade exit with iter: " + std::to_string(iter));
        return -1;
    }

    return 1;
}

void ScreenFadeTask::schedule() {
    if (_main) {
        gdi().setScreenTransparency(_fadeIn ? 0 : 100, GE_MAIN);
    }

    if (_sub) {
        gdi().setScreenTransparency(_fadeIn ? 0 : 100, GE_SUB);
    }

    taskCruncher().push(this);
}