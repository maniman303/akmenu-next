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
#include "sprite.h"
#include "bmp15.h"
#include "singleton.h"
#include "font.h"

#define SYSTEM_FONT_HEIGHT 12
#define COLOR u16

enum GRAPHICS_ENGINE { GE_MAIN = 0, GE_SUB = 1 };
enum MAIN_ENGINE_LAYER { MEL_UP = 0, MEL_MIDDLE = 1, MEL_DOWN = 2 };
enum SUB_ENGINE_LAYER { SEL_UP = 0, SEL_DOWN = 1 };

class cSprite;

class cGdi {
  public:
    cGdi();

    virtual ~cGdi();

  public:
    void init();
    void setScreenTransparency(u16 value, GRAPHICS_ENGINE engine);
    void setScreenTransparency(u16 value, bool light, GRAPHICS_ENGINE engine);
    u16 getScreenTransparency(GRAPHICS_ENGINE engine);
    void setMainLayerTransparency(u16 value, MAIN_ENGINE_LAYER layer);
    void drawPixel(u8 x, u8 y, GRAPHICS_ENGINE engine);
    void drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine);
    void drawRadiusLine(s16 sx, s16 sy, u16 width, u16 length, s16 degrees, u16 color, GRAPHICS_ENGINE engine);
    void frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);
    void frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine);
    void fillRect(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine);

    u16 colorizeColor(u16 grey, u16 tint, u16 light, u16 dark);
    u16 blendColors(u16 color, u16 dest, u16 src, u16 opacity);
    u32 blendColors32(u32 color, u32 dest, u32 src, u16 opacity);

    void fillRectBlend(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine, u16 opacity);
    void maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);
    void maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine, u16 color);
    void maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);
    void maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, s32 destW, s32 destH, GRAPHICS_ENGINE engine, u16 color);
    void bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);
    void bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, u16 repeats, GRAPHICS_ENGINE engine);
    void bitBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine);
    void bitBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, s32 destW, s32 destH, u16 repeats, GRAPHICS_ENGINE engine);

    void bitMainBackground(const void* src);
    void bitSubBackground(const void* src);

    u16 getPenColor(GRAPHICS_ENGINE engine) {
        if (GE_MAIN == engine) {
            return _penColor;
        }

        return _penColorSub;
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
        if (layer == MEL_MIDDLE) {
            _scheduleMainBackdrop = true;
        }

        _mainEngineLayer = layer;
        _layerPitch = layer * SCREEN_WIDTH * SCREEN_HEIGHT;
    }

    void setSubEngineLayer(SUB_ENGINE_LAYER layer) {
        _subEngineLayer = layer;
        _subLayerPitch = layer * SCREEN_WIDTH * SCREEN_HEIGHT;
    }

    void pushMainBackground();
    void pushSubBackground();
    void present();
    void presentMain();

  protected:
    void swapLCD(void);
    void activeFbMain(void);  // fb = frame buffer
    void activeFbSub(void);

  private:
    std::vector<u16> _penColorStack;
    u16 _penColor;
    u16 _penColorSub;
    u16 _transColor;
    u16* _workMain;
    u16* _bufferMain1;
    u16* _bufferMain3;
    MAIN_ENGINE_LAYER _mainEngineLayer;
    SUB_ENGINE_LAYER _subEngineLayer;
    u32 _layerPitch;
    u32 _subLayerPitch;
    u16* _workSub;
    u16* _bufferSub1;
    bool _scheduleMainBackground;
    bool _scheduleMainBackdrop;
    bool _scheduleSubBackground;
    std::vector<cSprite> _mainSprites;
    std::vector<cSprite> _subSprites;
};

typedef t_singleton<cGdi> cGdi_s;
inline cGdi& gdi() {
    return cGdi_s::instance();
}
