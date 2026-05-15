#pragma once

#include "singleton.h"

class cSystemDetails {
  public:
    cSystemDetails();
    void initArm7RegStatuses();
    void update();
    bool fifoStatus();
    bool isBatteryLow();
    bool isBatteryCharging();

  protected:
    bool _fifoInit;
    bool _chargingStatus;
};

typedef t_singleton<cSystemDetails> SystemDetails_s;
inline cSystemDetails& sd() {
    return SystemDetails_s::instance();
}