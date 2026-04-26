#include "screenfade.h"
#include "taskcruncher.h"

ScreenFadeTask::ScreenFadeTask() : ScreenFadeTask::ScreenFadeTask(true, true, true) {}

ScreenFadeTask::ScreenFadeTask(bool fadeIn) : ScreenFadeTask::ScreenFadeTask(true, true, fadeIn) {}

ScreenFadeTask::ScreenFadeTask(bool main, bool sub, bool fadeIn) {
    _main = main;
    _sub = sub;
    _fadeIn = fadeIn;
    _animation.setDuration(20);
}

s16 ScreenFadeTask::process(s16 iter) {
    if (_animation.isCompleted()) {
        return -1;
    }
    
    if (iter == 0 && !_animation.isPlaying()) {
        _animation.play();
    }

    u16 value = _animation.value();
    
    if (_main) {
        gdi().setScreenTransparency(value, GE_MAIN);
    }

    if (_sub) {
        gdi().setScreenTransparency(value, GE_SUB);
    }

    return 1;
}

void ScreenFadeTask::schedule() {
    if (_main) {
        gdi().setScreenTransparency(0, GE_MAIN);
    }

    if (_sub) {
        gdi().setScreenTransparency(0, GE_SUB);
    }

    _animation.play();

    taskCruncher().push(this);
}