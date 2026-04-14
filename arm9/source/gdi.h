/*
    gdi.h
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

#include <nds.h>
#include <vector>
#include "bmp15.h"
#include "singleton.h"
#include "font.h"

#define SYSTEM_FONT_HEIGHT 12
#define COLOR u16

enum GRAPHICS_ENGINE { GE_MAIN = 0, GE_SUB = 1 };

enum MAIN_ENGINE_LAYER { MEL_UP = 0, MEL_DOWN = 1 };

enum SUB_ENGINE_MODE { SEM_TEXT = 0, SEM_GRAPHICS = 1 };

class cSprite;

class cGdi {
  public:
    cGdi();

    virtual ~cGdi();

  public:
    void init();

    void drawPixel(u8 x, u8 y, GRAPHICS_ENGINE engine);

    void drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine);

    void drawRadiusLine(s16 sx, s16 sy, u16 width, u16 length, s16 degrees, u16 color, GRAPHICS_ENGINE engine);

    void frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);

    void frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine);

    void fillRect(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);

    u16 blendColors(u16 color, u16 dest, u16 src, u16 opacity);

    u32 blendColors32(u32 color, u32 dest, u32 src, u16 opacity);

    void fillRectBlend(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine,
                       u16 opacity);

    void maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH,
                 GRAPHICS_ENGINE engine);

    void maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH,
                 GRAPHICS_ENGINE engine, u16 color);

    void maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                 GRAPHICS_ENGINE engine);

    void maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                 GRAPHICS_ENGINE engine, u16 color);

    void bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH,
                GRAPHICS_ENGINE engine);

    void bitBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                GRAPHICS_ENGINE engine);

    u16 getPenColor(GRAPHICS_ENGINE engine) {
        if (GE_MAIN == engine)
            return _penColor & ~BIT(15);
        else
            return _penColorSub & ~BIT(15);
    }

    void setPenColor(u16 color, GRAPHICS_ENGINE engine) {
        if (GE_MAIN == engine)
            _penColor = color | BIT(15);
        else
            _penColorSub = color | BIT(15);
    }

    void setTransColor(u16 color) { _transColor = color | BIT(15); }

    s16  textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine);

    s16  textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont);

    s16  textOutRect(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine);

    s16  textOutRect(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont);

    void setMainEngineLayer(MAIN_ENGINE_LAYER layer) {
        _mainEngineLayer = layer;
        _layerPitch = layer * 256 * 192;
    }

    void present(GRAPHICS_ENGINE engine);

    void scheduleDrop();
#ifdef DEBUG
    void switchSubEngineMode();
#endif

  protected:
    void swapLCD(void);
    void activeFbMain(void);  // fb = frame buffer
    void activeFbSub(void);

  private:
    std::vector<u16> _penColorStack;
    u16 _penColor;
    u16 _penColorSub;
    u16 _transColor;
    u16* _bufferMain1;
    u16* _bufferMain2;
    u16* _bufferMain3;
    MAIN_ENGINE_LAYER _mainEngineLayer;
    SUB_ENGINE_MODE _subEngineMode;
    u32 _layerPitch;
    u16* _bufferSub1;
    u16* _bufferSub2;
#ifdef DEBUG
    u16* _bufferSub3;
#endif
    cSprite* _sprites;
    bool _scheduleDrop;
};

typedef t_singleton<cGdi> cGdi_s;
inline cGdi& gdi() {
    return cGdi_s::instance();
}
