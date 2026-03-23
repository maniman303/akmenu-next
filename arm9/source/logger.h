#pragma once

#include <string>
#include "singleton.h"

class cLogger {
  public:
    void init();

    void info(std::string message);
};

typedef t_singleton<cLogger> logger_s;
inline cLogger& logger() {
    return logger_s::instance();
}