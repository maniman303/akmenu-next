#include "taskworker.h"
#include "logger.h"

void TaskWorker::onCompleted() {
    if (_onCompleted) {
        _onCompleted();
    }
}

void TaskWorker::setOnCompleted(std::function<void()> onCompleted) {
    _onCompleted = onCompleted;        
}

bool TaskWorker::process() {
    logger().info("Task worker default process.");
    return true;
}