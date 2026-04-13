#include "image.h"

namespace akui {
    cImage::cImage(cWindow* parent) : cWindow(parent, "") {
        setEngine(GE_MAIN);
        _color = 0;
    }

    cImage::cImage(cWindow* parent, cSize size, u16 color) : cWindow(parent, "") {
        setEngine(GE_MAIN);
        setSize(size);

        if (color != 0) {
            _color = BIT(15) | color;
            _background = createBMP15(size.x, size.y, _color);
        }
    }

    cImage::~cImage() {}

    cWindow& cImage::loadAppearance(const std::string& aFileName) {
        _background = createBMP15FromFile(aFileName);
        if (!_background.valid() && _color != 0) {
            _background = createBMP15(size().x, size().y, _color);
        }

        setSize(cSize(_background.width(), _background.height()));

        return *this;
    }

    bool cImage::valid() {
        return _background.valid();
    }

    void cImage::draw(s32 x, s32 y) {
        if (!_background.valid()) {
            return;
        }

        const cPoint position = cPoint(x, y);
        cRect rect(position, position);
        gdi().bitBlt(_background.buffer(), x, y, size().x, size().y, selectedEngine());
    }

    void cImage::draw() {
        draw(position().x, position().y);
    }
}