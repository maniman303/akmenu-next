#include "smalldate.h"
#include "globalsettings.h"
#include "inifile.h"
#include "fontfactory.h"
#include "stringtool.h"
#include "systemfilenames.h"
#include "unicode.h"
#include "windowmanager.h"

cSmallDate::cSmallDate() : cWindow(NULL, "SmallDate") {
    _textColor = 0;
    _show = false;
    _engine = GE_SUB;
}

void cSmallDate::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _textColor = ini.GetInt("small date", "color", 0xFFFF);
    _show = ini.GetInt("small date", "show", _show);

    int dx = ini.GetInt("small date", "x", 0);
    int dy = ini.GetInt("small date", "y", 0);

    setPosition(cPoint(dx, dy));
}

void cSmallDate::draw() {
    if (!_show) {
        return;
    }

    int day = datetime().day();
    int month = datetime().month();

    int first = day;
    int second = month;

    if (gs().dateFormat > 0) {
        first = month;
        second = day;
    }
    
    const cFont& textFont = fontSecondary();
    u32 textWidth = textFont.TextWidth(formatString("%02d", first));
    u32 exampleWidth = textFont.TextWidth("22");
    int x = position().x + (exampleWidth - textWidth);

    gdi().setPenColor(_textColor, _engine);
    x = gdi().textOut(x, position().y, formatString("%02d", first).c_str(), _engine, textFont) + 1;
    x = gdi().textOut(x, position().y, "/", _engine, textFont) + 1;
    x = gdi().textOut(x, position().y, formatString("%02d", second).c_str(), _engine, textFont);
}