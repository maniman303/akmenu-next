#pragma once

#include <nds.h>
#include <vector>
#include "taskworker.h"

class WorkIndicator {
  public:
    virtual bool busy() const = 0;
};

class WorkIndicatorTask : public TaskWorker {
  public:
    WorkIndicatorTask(std::vector<WorkIndicator*> indicators, std::function<void()> onCompleted);

    s16 process(s16 iter) override;
    void schedule();
  private:
  //TODO: A pointer to window to disable / enable input
    std::vector<WorkIndicator*> _indicators;
};
