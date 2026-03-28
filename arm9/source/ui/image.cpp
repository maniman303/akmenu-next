#include "image.h"

namespace akui {
    cImage::cImage(cWindow* parent) : cWindow(parent, ""), _bitmapDesc() {}

    cImage::~cImage() {}

    cWindow& cImage::loadAppearance(const std::string& aFileName) {
        _bitmapDesc = cBitmapDesc();
        _bitmapDesc.loadData(aFileName);

        setSize(_bitmapDesc.size());

        return *this;
    }

    bool cImage::valid() {
        return _bitmapDesc.valid();
    }

    void cImage::draw(s32 x, s32 y) {
        if (!_bitmapDesc.valid()) {
            return;
        }

        const cPoint position = cPoint(x, y);
        cRect rect(position, position);
        _bitmapDesc.draw(rect, selectedEngine());
    }

    void cImage::draw() {
        draw(position().x, position().y);
    }
}