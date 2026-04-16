#include "analogclock.h"
#include "globalsettings.h"
#include "cachedinifile.h"
#include "datetime.h"
#include "systemfilenames.h"

cAnalogClock::cAnalogClock() {
    setEngine(GE_SUB);
    _show = false;
    _lengthSeconds = 0;
    _lengthMinutes = 0;
    _lengthHours = 0;
    _colorSeconds = 0;
    _colorMinutes = 0;
    _colorHours = 0;
    _colorDot = 0;
}

cAnalogClock::~cAnalogClock() {}

void cAnalogClock::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _show = ini.GetInt("analog clock", "show", _show);
    int dx = ini.GetInt("analog clock", "x", 0);
    int dy = ini.GetInt("analog clock", "y", 0);
    setRelativePosition(cPoint(dx, dy));
    _lengthSeconds = ini.GetInt("analog clock", "lengthSeconds", 0);
    _lengthMinutes = ini.GetInt("analog clock", "lengthMinutes", 0);
    _lengthHours = ini.GetInt("analog clock", "lengthHours", 0);
    _lengthAlarm = ini.GetInt("analog clock", "lengthAlarm", 0);
    _colorSeconds = ini.GetInt("analog clock", "colorSeconds", 0) | BIT(15);
    _colorMinutes = ini.GetInt("analog clock", "colorMinutes", 0) | BIT(15);
    _colorHours = ini.GetInt("analog clock", "colorHours", 0) | BIT(15);
    _colorAlarm = ini.GetInt("analog clock", "colorAlarm", 0) | BIT(15);
    _colorDot = ini.GetInt("analog clock", "colorDot", 0) | BIT(15);
}

void cAnalogClock::draw() {
    if (!_show) {
        return;
    }

    s16 degreesSeconds = datetime().seconds() * 6;
    s16 degreesMinutes = datetime().minutes() * 6;
    s16 degreesHours = (datetime().hours() % 12) * 30;
    s16 degreesAlarm = (PersonalData->alarmHour % 12) * 30;
    degreesAlarm += (PersonalData->alarmMinute / 2);

    gdi().drawRadiusLine(position().x, position().y, 2, _lengthSeconds, degreesSeconds, _colorSeconds, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthMinutes, degreesMinutes, _colorMinutes, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthHours, degreesHours, _colorHours, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthAlarm, degreesAlarm, _colorAlarm, selectedEngine());
    gdi().fillRect(_colorDot, _colorDot, position().x - 2, position().y - 2, 5, 5, selectedEngine());
}