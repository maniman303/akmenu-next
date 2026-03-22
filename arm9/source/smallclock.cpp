#include <chrono>
#include "smallclock.h"
#include "globalsettings.h"
#include "inifile.h"
#include "fontfactory.h"
#include "stringtool.h"
#include "systemfilenames.h"
#include "unicode.h"
#include "windowmanager.h"

cSmallClock::cSmallClock() : cWindow(NULL, "SmallClock") {
    _dx = 0;
    _dy = 0;
    _textColor = 0;
    _show = false;
    _showColon = true;

    _size = cSize(1, 1);
    _position = cPoint(0, 0);
    _engine = GE_SUB;
}

void cSmallClock::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _dx = ini.GetInt("small clock", "x", 0);
    _dy = ini.GetInt("small clock", "y", 0);
    _textColor = ini.GetInt("small clock", "color", 0xFFFF);
    _show = ini.GetInt("small clock", "show", _show);
}

void cSmallClock::draw() {
    if (!_show) {
        return;
    }

    int hours = datetime().hours();
    int minutes = datetime().minutes();
    
    const cFont& textFont = fontSecondary();
    u32 textWidth = textFont.TextWidth(formatString("%02d", hours));
    u32 exampleWidth = textFont.TextWidth("22");
    int x = _dx + exampleWidth - textWidth;

    std::string time = formatString("%02d : %02d", hours, minutes);

    gdi().setPenColor(_textColor, _engine);
    x = gdi().textOut(x, _dy, formatString("%02d", hours).c_str(), _engine, fontSecondary()) + 1;

    if (_showColon) {
        x = gdi().textOut(x, _dy, ":", _engine, fontSecondary()) + 1;
    } else {
        x += textFont.TextWidth(":") + 1;
    }
    
    x = gdi().textOut(x, _dy, formatString("%02d", minutes).c_str(), _engine, fontSecondary());
}

void cSmallClock::flipColon() {
    _showColon = !_showColon;
}