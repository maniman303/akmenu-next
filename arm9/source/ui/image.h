#pragma once

#include "window.h"
#include "bmp15.h"

namespace akui {
    class cImage : public cWindow {
      public:
        cImage(cWindow* parent);

        cImage(cWindow* parent, cSize size, u16 color);

        ~cImage() override;

        cWindow& loadAppearance(const std::string& aFileName) override;

        bool valid();

        void draw(s32 x, s32 y);

        void draw() override;

      protected:
        u16 _color;
        cBMP15 _background;
    };
}