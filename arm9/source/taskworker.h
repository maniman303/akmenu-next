#pragma once

#include <nds.h>
#include <functional>

class TaskWorker {
  public:
    void onCompleted();

    void setOnCompleted(std::function<void()> onCompleted);

    virtual bool process();

  private:
    std::function<void()> _onCompleted;
};