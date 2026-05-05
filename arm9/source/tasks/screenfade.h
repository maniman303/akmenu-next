#pragma once

#include <nds.h>
#include "taskworker.h"
#include "../gdi.h"
#include "../ui/animation.h"

class ScreenFadeTask : public TaskWorker {
  public:
    ScreenFadeTask();
    ScreenFadeTask(bool fadeIn);
    ScreenFadeTask(bool main, bool sub, bool fadeIn);

    void init();
    s16 process(s16 iter) override;
    void schedule();
  private:
    bool _main;
    bool _sub;
    bool _fadeIn;
    Animation _animation;
};