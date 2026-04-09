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
    struct TaskEntry {
      std::unique_ptr<TaskWorker> task;
      s16 iter;

      TaskEntry(std::unique_ptr<TaskWorker> t, s16 i)
          : task(std::move(t)), iter(i) {}
    };

    std::deque<TaskEntry> _deque;
};

typedef t_singleton<TaskCruncher> TaskCruncher_s;
inline TaskCruncher& taskCruncher() {
    return TaskCruncher_s::instance();
}