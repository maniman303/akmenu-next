#include "taskcruncher.h"
#include "logger.h"

TaskCruncher::TaskCruncher() {
    _deque = std::deque<TaskEntry>();
}

void TaskCruncher::push(std::unique_ptr<TaskWorker> task) {
    _deque.push_front(TaskEntry(std::move(task), 0));
}

void TaskCruncher::process() {
    if (_deque.empty()) {
        return;
    }

    TaskEntry& entry = _deque.front();
    entry.iter = entry.task->process(entry.iter);
    if (entry.iter >= 0) {
        return;
    }

    entry.task->onCompleted();
    _deque.pop_front();
}