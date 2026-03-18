#pragma once

#include "singleton.h"

class cSystemDetails {
  public:
    cSystemDetails();
    void initArm7RegStatuses();
    int batteryStatus();
    int volumeStatus();

  protected:
    bool _fifoInit;
};

typedef t_singleton<cSystemDetails> SystemDetails_s;
inline cSystemDetails& sd() {
    return SystemDetails_s::instance();
}