#include "analogclock.h"
#include "globalsettings.h"
#include "cachedinifile.h"
#include "datetime.h"
#include "personaldata.h"
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
    _colorSeconds = ini.GetColor("analog clock", "colorSeconds", 0);
    _colorMinutes = ini.GetColor("analog clock", "colorMinutes", 0);
    _colorHours = ini.GetColor("analog clock", "colorHours", 0);
    _colorAlarm = ini.GetColor("analog clock", "colorAlarm", 0);
    _colorDot = ini.GetColor("analog clock", "colorDot", 0);

    bool colorize = ini.GetInt("analog clock", "colorize", 0);
    if (colorize && _colorSeconds == 0x8000) {
        _colorSeconds = personalData().color();
    }

    if (colorize && _colorMinutes == 0x8000) {
        _colorMinutes = personalData().color();
    }

    if (colorize && _colorHours == 0x8000) {
        _colorHours = personalData().color();
    }

    if (colorize && _colorAlarm == 0x8000) {
        _colorAlarm = personalData().color();
    }

    if (colorize && _colorDot == 0x8000) {
        _colorDot = personalData().color();
    }
}

void cAnalogClock::draw() {
    if (!_show) {
        return;
    }

    s16 degreesSeconds = datetime().seconds() * 6;
    s16 degreesMinutes = datetime().minutes() * 6;
    s16 degreesHours = (datetime().hours() % 12) * 30;
    s16 degreesAlarm = (personalData().alarmHour() % 12) * 30;
    degreesAlarm += (personalData().alarmMinute() / 2);

    gdi().drawRadiusLine(position().x, position().y, 2, _lengthSeconds, degreesSeconds, _colorSeconds, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthMinutes, degreesMinutes, _colorMinutes, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthHours, degreesHours, _colorHours, selectedEngine());
    gdi().drawRadiusLine(position().x, position().y, 2, _lengthAlarm, degreesAlarm, _colorAlarm, selectedEngine());
    gdi().fillRect(_colorDot, _colorDot, position().x - 2, position().y - 2, 5, 5, selectedEngine());
}