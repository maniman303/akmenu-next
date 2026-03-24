#include "taskcruncher.h"
#include "logger.h"

TaskCruncher::TaskCruncher() {
    _deque = std::deque<std::unique_ptr<TaskWorker>>();
}

void TaskCruncher::push(std::unique_ptr<TaskWorker> task) {
    _deque.push_front(std::move(task));
}

void TaskCruncher::process() {
    if (_deque.empty()) {
        return;
    }

    std::unique_ptr<TaskWorker>& task = _deque.front();
    if (!task->process()) {
        return;
    }

    task->onCompleted();
    _deque.pop_front();
}