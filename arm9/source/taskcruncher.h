#pragma once

#include <nds.h>
#include <deque>
#include <memory>
#include "singleton.h"
#include "taskworker.h"

class TaskCruncher {
  public:
    TaskCruncher();

    void push(TaskWorker* task);

    void process();

  private:
    struct TaskEntry {
      std::unique_ptr<TaskWorker> task;
      s16 iter;

      TaskEntry(TaskWorker* t, s16 i)
          : task(t), iter(i) {}

      TaskEntry(TaskEntry&&) = default;
      TaskEntry& operator=(TaskEntry&&) = default;

      TaskEntry(const TaskEntry&) = delete;
      TaskEntry& operator=(const TaskEntry&) = delete;
    };

    std::deque<TaskEntry> _deque;
};

typedef t_singleton<TaskCruncher> TaskCruncher_s;
inline TaskCruncher& taskCruncher() {
    return TaskCruncher_s::instance();
}