#include "taskcruncher.h"
#include "logger.h"

TaskCruncher::TaskCruncher() {
    _deque = std::deque<TaskEntry>();
}

void TaskCruncher::push(TaskWorker* task) {
    if (task == NULL) {
        logger().error("Pushing NULL task.");
        return;
    }

    _deque.push_front(std::move(TaskEntry(task, 0)));
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