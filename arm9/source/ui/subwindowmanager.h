#pragma once

#include <nds.h>
#include "singleton.h"

class cSubWindowManager {
  public:
    cSubWindowManager();
    ~cSubWindowManager();

    void blink();
    void update();
  private:
    void updateBackground();
};

typedef t_singleton<cSubWindowManager> subWindowManager_s;
inline cSubWindowManager& subWindowManager() {
    return subWindowManager_s::instance();
}