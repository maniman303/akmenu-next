#pragma once

#include <string>
#include "singleton.h"

class cLogger {
  public:
    void init();
    void info(std::string message);
    void error(std::string message);

  private:
    void write(std::string prefix, std::string message);
};

typedef t_singleton<cLogger> logger_s;
inline cLogger& logger() {
    return logger_s::instance();
}