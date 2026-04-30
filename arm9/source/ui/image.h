#pragma once

#include "window.h"
#include "bmp15.h"

namespace akui {
    class cImage : public cWindow {
      public:
        cImage(cWindow* parent);

        cImage(cWindow* parent, cSize size, u16 color);

        cImage(cWindow* parent, cSize size, u16 color, bool hasAlpha);

        ~cImage() override;

        void loadAppearance(const std::string& aFileName);

        bool valid();

        void setAlpha(bool hasAlpha);

        void draw(s32 x, s32 y);

        void draw(s32 x, s32 y, u16 repeats);

        void draw() override;

      protected:
        u16 _color;
        cBMP15 _background;
        bool _hasAlpha;
    };
}