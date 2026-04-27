#pragma once

#include <nds.h>
#include <string>
#include "singleton.h"

class cPersonalData {
  public:
    std::string username();
    u8 alarmHour();
    u8 alarmMinute();
    u16 color();
};

typedef t_singleton<cPersonalData> PersonalData_s;
inline cPersonalData& personalData() {
    return PersonalData_s::instance();
}