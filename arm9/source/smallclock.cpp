#include <chrono>
#include "smallclock.h"
#include "globalsettings.h"
#include "cachedinifile.h"
#include "fontfactory.h"
#include "stringtool.h"
#include "systemfilenames.h"
#include "unicode.h"
#include "windowmanager.h"

cSmallClock::cSmallClock() : cWindow(NULL, "SmallClock") {
    _textColor = 0;
    _show = false;
    _showColon = true;
    _engine = GE_SUB;
}

void cSmallClock::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _textColor = ini.GetInt("small clock", "color", 0xFFFF);
    _show = ini.GetInt("small clock", "show", _show);
    int dx = ini.GetInt("small clock", "x", 0);
    int dy = ini.GetInt("small clock", "y", 0);

    setRelativePosition(cPoint(dx, dy));
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
    int x = position().x + exampleWidth - textWidth;

    std::string time = formatString("%02d : %02d", hours, minutes);

    gdi().setPenColor(_textColor, _engine);
    x = gdi().textOut(x, position().y, formatString("%02d", hours).c_str(), _engine, fontSecondary()) + 1;

    if (_showColon) {
        x = gdi().textOut(x, position().y, ":", _engine, fontSecondary()) + 1;
    } else {
        x += textFont.TextWidth(":") + 1;
    }
    
    x = gdi().textOut(x, position().y, formatString("%02d", minutes).c_str(), _engine, fontSecondary());
}

void cSmallClock::flipColon() {
    _showColon = !_showColon;
}