#include "fpscounter.h"
#include "systemfilenames.h"
#include "cachedinifile.h"
#include "timer.h"
#include "stringtool.h"
#include "fontfactory.h"

cFpsCounter::cFpsCounter() : cWindow(NULL, "FpsCounter") {
    _textColor = 0;
    _show = false;
    _engine = GE_SUB;
}

void cFpsCounter::init() {
    CIniFile ini = iniFiles().get(SFN_UI_SETTINGS);
    _textColor = ini.GetInt("fps counter", "color", 0xFFFF);
    _show = ini.GetInt("fps counter", "show", _show);
    int dx = ini.GetInt("fps counter", "x", 0);
    int dy = ini.GetInt("fps counter", "y", 0);

    setRelativePosition(cPoint(dx, dy));
}

void cFpsCounter::draw() {
    gdi().textOut(position().x, position().y, formatString("FPS: %02d", timer().getFps()).c_str(), _engine, fontSecondary());
}