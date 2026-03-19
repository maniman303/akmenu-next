#include "image.h"

namespace akui {
    cImage::cImage(cWindow* parent) : cWindow(parent, ""), _bitmapDesc(NULL) {}

    cImage::~cImage() {
        if (_bitmapDesc != NULL) {
            delete _bitmapDesc;
        }
    }

    cWindow& cImage::loadAppearance(const std::string& aFileName) {
        if (_bitmapDesc != NULL) {
            delete _bitmapDesc;
            _bitmapDesc = NULL;
        }

        _bitmapDesc = new cBitmapDesc();
        _bitmapDesc->loadData(aFileName);

        return *this;
    }

    bool cImage::valid() {
        if (_bitmapDesc == NULL) {
            return false;
        }

        return _bitmapDesc->valid();
    }

    void cImage::draw(s32 x, s32 y) {
        if (_bitmapDesc == NULL) {
            _bitmapDesc = new cBitmapDesc();
        }

        const cPoint position = cPoint(x, y);
        cRect rect(position, position);
        _bitmapDesc->draw(rect, selectedEngine());
    }

    void cImage::draw() {
        draw(position().x, position().y);
    }
}