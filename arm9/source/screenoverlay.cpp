#include "screenoverlay.h"

cScreenOverlay::cScreenOverlay() {
    setEngine(GE_MAIN);
    _background = createBMP15(SCREEN_WIDTH, SCREEN_HEIGHT, 0xffffffff);
}

void cScreenOverlay::draw() {
    gdi().maskBlt(_background.buffer(), 0, 0, _background.width(), _background.height(), selectedEngine());
}