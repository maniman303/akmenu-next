#include "image.h"

namespace akui {
    cImage::cImage(cWindow* parent) : cWindow(parent, ""), _bitmapDesc(NULL) {}

    cImage::~cImage() {
        if (_bitmapDesc) {
            delete _bitmapDesc;
        }
    }

    cWindow& cImage::loadAppearance(const std::string& aFileName) {
        _bitmapDesc = new cBitmapDesc();
        _bitmapDesc->loadData(aFileName);

        return *this;
    }

    void cImage::draw(s32 x, s32 y) {
        if (_bitmapDesc == NULL) {
            _bitmapDesc = new cBitmapDesc();
        }

        const cPoint position = cPoint(x, y);
        cRect rect(position, position);
        _bitmapDesc->draw(rect, selectedEngine());
    }

    void cImage::draw() { }
}