#include "smalldate.h"
#include "globalsettings.h"
#include "inifile.h"
#include "fontfactory.h"
#include "stringtool.h"
#include "systemfilenames.h"
#include "unicode.h"
#include "windowmanager.h"

cSmallDate::cSmallDate() : cWindow(NULL, "SmallDate") {
    _dx = 0;
    _dy = 0;
    _textColor = 0;
    _show = false;

    _size = cSize(1, 1);
    _position = cPoint(0, 0);
    _engine = GE_SUB;
}

void cSmallDate::init() {
    CIniFile ini(SFN_UI_SETTINGS);
    _dx = ini.GetInt("small date", "x", 0);
    _dy = ini.GetInt("small date", "y", 0);
    _textColor = ini.GetInt("small date", "color", 0xFFFF);
    _show = ini.GetInt("small date", "show", _show);
}

void cSmallDate::draw() {
    if (!_show) {
        return;
    }

    int day = datetime().day();
    int month = datetime().month();
    
    const cFont& textFont = fontSecondary();
    u32 dayWidth = textFont.TextLenght(formatString("%02d", day));
    u32 exampleWidth = textFont.TextLenght("22");
    int xOffset = exampleWidth - dayWidth;

    std::string date = formatString("%02d / %02d", day, month);

    gdi().setPenColor(_textColor, _engine);
    gdi().textOut(_dx + xOffset, _dy, date.c_str(), _engine, textFont);
}