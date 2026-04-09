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

s16 TaskWorker::process(s16 iter) {
    logger().info("Task worker default process.");
    return -1;
}