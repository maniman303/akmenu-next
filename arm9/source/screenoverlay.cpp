#include "screenoverlay.h"

cScreenOverlay::cScreenOverlay() {
    setEngine(GE_MAIN);
    _background = createBMP15((u16)SCREEN_WIDTH, (u16)SCREEN_HEIGHT, 0xffff);
}

cWindow& cScreenOverlay::loadAppearance(const std::string& aFileName) {
    _background = createBMP15FromFile(aFileName);
    if (!_background.valid()) {
        _background = createBMP15((u16)SCREEN_WIDTH, (u16)SCREEN_HEIGHT, 0xffff);
    }
    
    return *this;
}

void cScreenOverlay::draw() {
    gdi().bitBlt(_background.buffer(), 0, 0, _background.width(), _background.height(), selectedEngine());
}