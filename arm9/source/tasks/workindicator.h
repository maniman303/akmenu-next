#pragma once

#include <nds.h>
#include <vector>
#include "taskworker.h"
#include "window.h"

class WorkIndicator {
  public:
    virtual ~WorkIndicator() {} 
    virtual bool busy() const = 0;
};

class WorkIndicatorTask : public TaskWorker {
  public:
    WorkIndicatorTask(std::vector<WorkIndicator*> indicators, cWindow* window, std::function<void()> onCompleted);

    s16 process(s16 iter) override;
    void schedule();
  private:
    cWindow* _window;
    std::vector<WorkIndicator*> _indicators;
};
