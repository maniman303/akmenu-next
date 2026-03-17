#pragma once

#include "window.h"
#include "bitmapdesc.h"

namespace akui {
    class cImage : public cWindow {
      public:
        cImage(cWindow* parent);

        ~cImage();

        cWindow& loadAppearance(const std::string& aFileName);

        void draw(s32 x, s32 y);

        void draw();

      protected:
        cBitmapDesc* _bitmapDesc;
    };
}