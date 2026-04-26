#pragma once

#include <nds.h>
#include <functional>

class TaskWorker {
  public:
    void onCompleted();

    void setOnCompleted(std::function<void()> onCompleted);

    virtual s16 process(s16 iter);

  protected:
    std::function<void()> _onCompleted;
};