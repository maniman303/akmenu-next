#include "workindicator.h"
#include "taskcruncher.h"
#include "logger.h"

WorkIndicatorTask::WorkIndicatorTask(std::vector<WorkIndicator*> indicators, std::function<void()> onCompleted) {
    _indicators = indicators;
    _onCompleted = onCompleted;
}

s16 WorkIndicatorTask::process(s16 iter) {
    for (const WorkIndicator* indicator : _indicators) {
        if (indicator->busy()) {
            return 1;
        }
    }
    
    return -1;
}

void WorkIndicatorTask::schedule() {
    taskCruncher().push(this);
}