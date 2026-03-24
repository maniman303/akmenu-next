#pragma once

#include <nds.h>
#include <deque>
#include <memory>
#include "singleton.h"
#include "taskworker.h"

class TaskCruncher {
  public:
    TaskCruncher();

    void push(std::unique_ptr<TaskWorker> task);

    void process();

  private:
    std::deque<std::unique_ptr<TaskWorker>> _deque;
};

typedef t_singleton<TaskCruncher> TaskCruncher_s;
inline TaskCruncher& taskCruncher() {
    return TaskCruncher_s::instance();
}