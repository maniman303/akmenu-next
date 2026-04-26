#include "workindicator.h"
#include "taskcruncher.h"
#include "logger.h"

WorkIndicatorTask::WorkIndicatorTask(std::vector<WorkIndicator*> indicators, cWindow* window, std::function<void()> onCompleted) {
    _indicators = indicators;
    _window = window;
    _onCompleted = onCompleted;
}

s16 WorkIndicatorTask::process(s16 iter) {
    for (const WorkIndicator* indicator : _indicators) {
        if (indicator->busy()) {
            if (_window != NULL) {
                _window->disableInput();
            }

            return 1;
        }
    }

    if (_window != NULL) {
        _window->enableInput();
    }
    
    return -1;
}

void WorkIndicatorTask::schedule() {
    taskCruncher().push(this);
}