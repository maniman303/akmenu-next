/*
    gdi.cpp
    Copyright (C) 2007 Acekard, www.acekard.com
    Copyright (C) 2007-2009 somebody
    Copyright (C) 2009 yellow wood goblin

    SPDX-License-Identifier: GPL-3.0-or-later
*/

#include "gdi.h"
#include <cstdio>
#include <cstring>
#include <string>
#include <math.h>
#include "../../share/memtool.h"
#include "dbgtool.h"
#include "fontfactory.h"
#include "globalsettings.h"
#include "sprite.h"
#include "userinput.h"
#include "logger.h"

static inline void dmaCopyWordsGdi(uint8 channel, const void* src, void* dest, uint32 size) {
    DC_FlushRange(src, size);
    dmaCopyWords(channel, src, dest, size);
    DC_InvalidateRange(dest, size);
}

#ifdef DEBUG
PrintConsole custom_console;

static void MyInitConsole(u16* aBufferSub1, u16* aBufferSub2) {
    custom_console = *consoleGetDefault();

    custom_console.loadGraphics = false;

    consoleInit(&custom_console, custom_console.bgLayer, BgType_Text4bpp, BgSize_T_256x256,
                custom_console.mapBase, custom_console.gfxBase, false, false);

    custom_console.fontBgMap = aBufferSub1;
    custom_console.fontBgGfx = aBufferSub2;

    dmaCopy(custom_console.font.gfx, custom_console.fontBgGfx,
            custom_console.font.numChars * 64 / 2);
    custom_console.fontCurPal = 15 << 12;

    u16* palette = BG_PALETTE_SUB;
    palette[1 * 16 - 15] = RGB15(0, 0, 0);    // 30 normal black
    palette[2 * 16 - 15] = RGB15(15, 0, 0);   // 31 normal red
    palette[3 * 16 - 15] = RGB15(0, 15, 0);   // 32 normal green
    palette[4 * 16 - 15] = RGB15(15, 15, 0);  // 33 normal yellow

    palette[5 * 16 - 15] = RGB15(0, 0, 15);    // 34 normal blue
    palette[6 * 16 - 15] = RGB15(15, 0, 15);   // 35 normal magenta
    palette[7 * 16 - 15] = RGB15(0, 15, 15);   // 36 normal cyan
    palette[8 * 16 - 15] = RGB15(24, 24, 24);  // 37 normal white

    palette[9 * 16 - 15] = RGB15(15, 15, 15);  // 40 bright black
    palette[10 * 16 - 15] = RGB15(31, 0, 0);   // 41 bright red
    palette[11 * 16 - 15] = RGB15(0, 31, 0);   // 42 bright green
    palette[12 * 16 - 15] = RGB15(31, 31, 0);  // 43 bright yellow

    palette[13 * 16 - 15] = RGB15(0, 0, 31);    // 44 bright blue
    palette[14 * 16 - 15] = RGB15(31, 0, 31);   // 45 bright magenta
    palette[15 * 16 - 15] = RGB15(0, 31, 31);   // 46 bright cyan
    palette[16 * 16 - 15] = RGB15(31, 31, 31);  // 47 & 39 bright white
}
#endif

cGdi::cGdi() {
    _transColor = 0;
    _mainEngineLayer = MEL_UP;
    _subEngineMode = SEM_TEXT;
    _bufferMain2 = NULL;
    _bufferSub2 = NULL;
#ifdef DEBUG
    _bufferSub3 = NULL;
#endif
    _sprites = NULL;
    _scheduleDrop = false;
}

cGdi::~cGdi() {
    if (NULL != _bufferMain2) delete[] _bufferMain2;
    if (NULL != _bufferSub2) delete[] _bufferSub2;
#ifdef DEBUG
    if (NULL != _bufferSub3) delete[] _bufferSub3;
#endif
    if (NULL != _sprites) delete[] _sprites;
}

void cGdi::init() {
    swapLCD();
    activeFbMain();
    activeFbSub();
    cSprite::sysinit();
}

void cGdi::swapLCD(void) {
    lcdSwap();
}

void cGdi::activeFbMain(void) {
    vramSetBankB(VRAM_B_MAIN_BG_0x06000000);
    vramSetBankD(VRAM_D_MAIN_BG_0x06020000);

    vramSetBankA(VRAM_A_MAIN_SPRITE_0x06400000);

    REG_BG2CNT = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA = 1 << 8;  // 2 =放大倍数
    REG_BG2PD = 1 << 8;  // 2 =放大倍数
    REG_BG2PB = 0;
    REG_BG2PC = 0;
    REG_BG2Y = 0;
    REG_BG2X = 0;

    REG_BG3CNT = BG_BMP16_256x256 | BG_BMP_BASE(8) | BG_PRIORITY_2;
    REG_BG3PA = 1 << 8;  // 2 =放大倍数
    REG_BG3PD = 1 << 8;  // 2 =放大倍数
    REG_BG3PB = 0;
    REG_BG3PC = 0;
    REG_BG3Y = 0;
    REG_BG3X = 0;

    _bufferMain1 = (u16*)0x06000000;
    _bufferMain2 = (u16*)new u16[256 * 192 * 2];
    _bufferMain3 = (u16*)0x06020000;

    setMainEngineLayer(MEL_UP);

    zeroMemory(_bufferMain1, 0x20000);
    fillMemory(_bufferMain3, 0x20000, 0xffffffff);

    // REG_BLDCNT = BLEND_ALPHA | BLEND_DST_BG2 | BLEND_DST_BG3;
    // REG_BLDALPHA = (4 << 8) | 7;

    swiWaitForVBlank();  // remove tearing at bottop screen
    videoSetMode(MODE_5_2D | DISPLAY_BG2_ACTIVE | DISPLAY_BG3_ACTIVE | DISPLAY_SPR_ACTIVE |
                 DISPLAY_SPR_1D_BMP_SIZE_128 | DISPLAY_SPR_1D_BMP);
}

void cGdi::activeFbSub(void) {
    vramSetBankC(VRAM_C_SUB_BG_0x06200000);  // 128k
    _subEngineMode = SEM_GRAPHICS;

    // BMP bg 的参数设置，从 VRAM地址 0x06200000 开始，优先级3
    REG_BG2CNT_SUB = BG_BMP16_256x256 | BG_BMP_BASE(0) | BG_PRIORITY_1;
    REG_BG2PA_SUB = 1 << 8;
    REG_BG2PD_SUB = 1 << 8;
    REG_BG2PB_SUB = 0;
    REG_BG2PC_SUB = 0;
    REG_BG2Y_SUB = 0;
    REG_BG2X_SUB = 0;

    _bufferSub1 = (u16*)0x06200000;
    _bufferSub2 = (u16*)new u32[256 * 192 / 2];

    fillMemory(_bufferSub2, 0x18000, 0xffffffff);
    fillMemory(_bufferSub1, 0x18000, 0xffffffff);

    swiWaitForVBlank();
    videoSetModeSub(MODE_5_2D | DISPLAY_BG2_ACTIVE);
}

void cGdi::drawLine(s16 x1, s16 y1, s16 x2, s16 y2, GRAPHICS_ENGINE engine) {
    if ((x1 == x2) && (y1 == y2)) return;

    if (x1 == x2) {
        int ys, ye;
        if (y1 < y2) {
            ys = y1;
            ye = y2 - 1;
        } else {
            ys = y2 + 1;
            ye = y1;
        }
        for (int py = ys; py <= ye; py++) {
            drawPixel(x1, py, engine);
        }
        return;
    }

    if (y1 == y2) {
        int xs, xe;
        if (x1 < x2) {
            xs = x1;
            xe = x2 - 1;
        } else {
            xs = x2 + 1;
            xe = x1;
        }
        if (GE_MAIN == engine)
            fillRect(_penColor, _penColor, xs, y1, xe - xs + 1, 1, engine);
        else
            fillRect(_penColorSub, _penColorSub, xs, y1, xe - xs + 1, 1, engine);
        return;
    }

    if (abs(x2 - x1) > abs(y2 - y1)) {
        int px = 0;
        float py = 0;
        int xe = x2 - x1;
        float ye = y2 - y1;
        int xv;
        float yv;

        if (0 < xe) {
            xv = 1;
        } else {
            xv = -1;
        }
        yv = ye / abs(xe);

        while (px != xe) {
            drawPixel(x1 + px, y1 + (int)py, engine);
            px += xv;
            py += yv;
        }
        return;
    } else {
        float px = 0;
        int py = 0;
        float xe = x2 - x1;
        int ye = y2 - y1;
        float xv;
        int yv;

        xv = xe / abs(ye);
        if (0 < ye) {
            yv = 1;
        } else {
            yv = -1;
        }

        while (py != ye) {
            drawPixel(x1 + (int)px, y1 + py, engine);
            px += xv;
            py += yv;
        }
        return;
    }
}

static inline void putScreenPixel(u16* buffer, int x, int y, u16 color) {
    if (x < 0 || x >= SCREEN_WIDTH || y < 0 || y >= SCREEN_HEIGHT) {
        return;
    }

    buffer[y * 256 + x] = color;
}

void cGdi::drawRadiusLine(s16 sx, s16 sy, u16 width, u16 length, s16 degrees, u16 color, GRAPHICS_ENGINE engine) {
    static const double PI = 3.14159265358979323846;

    if (length <= width || width <= 1) {
        return;
    }

    length -= width;

    color = color | BIT(15);

    u16* buffer = engine == GE_MAIN ? _bufferMain2 + _layerPitch : _bufferSub2;

    degrees = (90 - degrees) % 360;
    double radius = degrees * PI / 180.0;

    s16 dx = sx + static_cast<s16>(length * std::cos(radius));
    s16 dy = sy - static_cast<s16>(length * std::sin(radius));

    int dx_abs = abs(dx - sx);
    int dy_abs = abs(dy - sy);

    int step_x = (sx < dx) ? 1 : -1;
    int step_y = (sy < dy) ? 1 : -1;

    int err = dx_abs - dy_abs;

    int xDir = -1;
    int yDir = -1;

    if (dx > sx || (dx >= sx && dy < sy)) {
        // sy -= (width - 1);
        // dy -= (width - 1);
        xDir = 1;
    }

    if (dy > sy || (dy >= sy && dx < sx)) {
        // sx -= (width - 1);
        // dx -= (width - 1);
        yDir = 1;
    }

    int x = sx;
    int y = sy;

    while (true) {
        for (int i = 0; i < width; i++) {
            for (int k = 0; k < width; k++) {
                putScreenPixel(buffer, x + xDir * i, y + yDir * k, color);
            }
        }

        if (x == dx && y == dy) break;

        int e2 = 2 * err;

        if (e2 > -dy_abs) {
            err -= dy_abs;
            x += step_x;
        }

        if (e2 < dx_abs) {
            err += dx_abs;
            y += step_y;
        }
    }
}

void cGdi::frameRect(s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine) {
    drawLine(x, y, x + w - 1, y, engine);
    drawLine(x + w - 1, y, x + w - 1, y + h - 1, engine);
    drawLine(x + w - 1, y + h - 1, x, y + h - 1, engine);
    drawLine(x, y + h - 1, x, y, engine);
}

void cGdi::frameRect(s16 x, s16 y, u16 w, u16 h, u16 thickness, GRAPHICS_ENGINE engine) {
    for (size_t ii = 0; ii < thickness; ++ii) {
        frameRect(x, y, w, h, engine);
        if (h <= 2 || w <= 2) break;
        ++x;
        ++y;
        w -= 2;
        h -= 2;
    }
}

void cGdi::fillRect(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine) {
    ALIGN(4) u16 color[2] = { (u16)(BIT(15) | color1), (u16)(BIT(15) | color2) };
    ALIGN(4) u16 altColor[2] = { (u16)(BIT(15) | color2), (u16)(BIT(15) | color1) };
    u16* pSrc = (u16*)color;
    u16* pAltSrc = (u16*)altColor;
    u16* pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (y << 8) + x + _layerPitch;
    else
        pDest = _bufferSub2 + (y << 8) + x;

    bool destAligned = !(x & 1);

    u16 destInc = 256 - w;
    u16 halfWidth = w >> 1;
    u16 remain = w & 1;

    if (destAligned)
        for (u32 i = 0; i < h; ++i) {
            u16* source = (i & 1) ? pSrc : pAltSrc;
            swiFastCopy(source, pDest, COPY_MODE_WORD | COPY_MODE_FILL | halfWidth);
            pDest += halfWidth << 1;
            if (remain) {
                *pDest++ = *source;
            }

            pDest += destInc;
        }
    else
        for (u32 i = 0; i < h; ++i) {
            for (u32 j = 0; j < w; ++j) {
                *pDest++ = pSrc[(j ^ i) & 1];
            }
            pDest += destInc;
        }
}

u16 cGdi::blendColors(u16 color, u16 dest, u16 src, u16 opacity) {
    if (opacity >= 100) {
        return color;
    }

    if ((dest & 0x8000) != 0) {
        src = dest;
    }

    if (color == 0 || opacity <= 0) {
        return src;
    }

    const u32 alpha  = (static_cast<u32>(opacity) * 655u) >> 11u;
    const u32 invAlpha = 32u - alpha;

    const u32 rb = ((static_cast<u32>(color) & 0x7c1fu) * alpha
                  + (static_cast<u32>(src)   & 0x7c1fu) * invAlpha) & 0xf83e0u;
    const u32 g  = ((static_cast<u32>(color) & 0x3e0u)  * alpha
                  + (static_cast<u32>(src)   & 0x3e0u)  * invAlpha) & 0x7c00u;

    return static_cast<u16>(((rb | g) >> 5u) | BIT(15));
}

void cGdi::fillRectBlend(u16 color1, u16 color2, s16 x, s16 y, u16 w, u16 h, GRAPHICS_ENGINE engine, u16 opacity) {
    if (opacity <= 0) {
        return;
    }

    if (opacity >= 100) {
        fillRect(color1, color2, x, y, w, h, engine);
        return;
    }

    if (color1 == 0 || color2 == 0) {
        return;
    }

    u16* pSrc = ((GE_MAIN == engine) ? _bufferMain3 : _bufferSub2) + (y << 8) + x;
    u16* pDest = ((GE_MAIN == engine) ? (_bufferMain2 + _layerPitch) : _bufferSub2) + (y << 8) + x;
    u32 destInc = 256 - w;
    for (u32 ii = 0; ii < h; ++ii) {
        for (u32 jj = 0; jj < w; ++jj) {
            u16 color = (jj & 1) ? color2 : color1;
            *pDest = blendColors(color, *pDest, *pSrc, opacity);

            pDest++;
            pSrc++;
        }

        pDest += destInc;
        pSrc += destInc;
    }
}

void cGdi::bitBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                  GRAPHICS_ENGINE engine) {
    if (destW <= 0) return;

    u16* pSrc = (u16*)src;
    u16* pDest = NULL;

    if (GE_MAIN == engine)
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    else
        pDest = _bufferSub2 + (destY)*256 + destX;

    bool destAligned = !(destX & 1);

    if (destW > srcW) destW = srcW;
    if (destH > srcH) destH = srcH;

    u16 srcInc = srcW - destW;
    u16 destInc = 256 - destW;
    u16 destHalfWidth = destW >> 1;
    u16 lineSize = destW << 1;
    u16 remain = destW & 1;

    if (destAligned) {
        for (u32 i = 0; i < destH; ++i) {
            dmaCopyWordsGdi(3, pSrc, pDest, lineSize);
            pDest += destHalfWidth << 1;
            pSrc += destHalfWidth << 1;
            if (remain) *pDest++ = *pSrc++;
            pDest += destInc;
            pSrc += srcInc;
        }
    }
}

void cGdi::bitBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine) {
    u16* pSrc = (u16*)src;
    u16* pDest = (engine == GE_MAIN) ? 
                    (_bufferMain2 + (destY * 256) + destX + _layerPitch) : 
                    (_bufferSub2 + (destY * 256) + destX);

    u16 pitchPixel = (destW + (destW & 1));
    u16 destInc = 256 - pitchPixel;
    u16 halfPitch = pitchPixel >> 1;

    bool destAligned = !(destX & 1);
    if (!destAligned) {
        for (u16 i = 0; i < destH; ++i) {
            swiCopy(pSrc, pDest, COPY_MODE_COPY | pitchPixel);

            pDest += pitchPixel;
            pSrc += pitchPixel;
            pDest += destInc;
        }

        return;
    }

    for (u16 i = 0; i < destH; ++i) {
        swiFastCopy(pSrc, pDest, COPY_MODE_WORD | COPY_MODE_COPY | halfPitch);

        pDest += halfPitch << 1;
        pSrc += halfPitch << 1;
        pDest += destInc;
    }
}

void cGdi::maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine) {
    return maskBlt(src, destW, destH, destX, destY, destW, destH, engine, 0);
}

void cGdi::maskBlt(const void* src, s16 destX, s16 destY, u16 destW, u16 destH, GRAPHICS_ENGINE engine, u16 color) {
    return maskBlt(src, destW, destH, destX, destY, destW, destH, engine, color);
}

void cGdi::maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                   GRAPHICS_ENGINE engine) {
    return maskBlt(src, srcW, srcH, destX, destY, destW, destH, engine, 0);
 }

void cGdi::maskBlt(const void* src, s16 srcW, s16 srcH, s16 destX, s16 destY, u16 destW, u16 destH,
                   GRAPHICS_ENGINE engine, u16 color) {
    if (destW <= 0) {
        return;
    }

    if (color != 0) {
        color = BIT(15) | color;
    }

    u16* pSrc = (u16*)src;
    u16* pDest = NULL;
    
    if (GE_MAIN == engine) {
        pDest = _bufferMain2 + (destY)*256 + destX + _layerPitch;
    } else {
        pDest = _bufferSub2 + (destY)*256 + destX;
    }

    if (destW > srcW) destW = srcW;
    if (destH > srcH) destH = srcH;

    u16 srcInc = srcW - destW;
    u16 pitch = (destW + (destW & 1));
    u16 destInc = 256 - pitch;
    u16 halfPitch = pitch >> 1;
    bool destAligned = !(destX & 1);

    if (destAligned) {
        for (u32 i = 0; i < destH; ++i) {
            for (u32 j = 0; j < halfPitch; ++j) {
                if (((*(u32*)pSrc) & 0x80008000) == 0x80008000) {
                    u32 newValue = (color == 0 ? *(u32*)pSrc : (((u32)color << 16) | color));
                    *(u32*)pDest = newValue;
                    pSrc += 2;
                    pDest += 2;
                } else {
                    if (*pSrc & 0x8000) *pDest = (color == 0 ? *pSrc : color);
                    pSrc++;
                    pDest++;
                    if (*pSrc & 0x8000) *pDest = (color == 0 ? *pSrc : color);
                    pSrc++;
                    pDest++;
                }
            }
            pDest += destInc;
            pSrc += srcInc;
        }
    } else {
        for (u16 i = 0; i < destH; ++i) {
            for (u16 j = 0; j < pitch; ++j) {
                if (*pSrc & 0x8000) *pDest = color == 0 ? *pSrc : color;
                pDest++;
                pSrc++;
            }
            pDest += destInc;
            pSrc += srcInc;
        }
    }
}

s16 cGdi::textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine) {
    return textOutRect(x, y, 256, 192, text, engine, font());
}

s16  cGdi::textOut(s16 x, s16 y, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont) {
    return textOutRect(x, y, 256, 192, text, engine, textFont);
}

s16  cGdi::textOutRect(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine) {
    return textOutRect(x, y, w, h, text, engine, font());
}

s16 cGdi::textOutRect(s16 x, s16 y, u16 w, u16 h, const char* text, GRAPHICS_ENGINE engine, const cFont& textFont) {
    u8 fontHeight = textFont.GetHeight();
    u8 fontDescend = textFont.GetDescend();

    const s16 originX = x, limitY = y + h - fontHeight;
    while (*text) {
        if ('\r' == *text || '\n' == *text) {
            y += (fontHeight + fontDescend);
            x = originX;
            ++text;
            if (y > limitY) break;
        } else {
            u32 ww, add;
            textFont.Info(text, &ww, &add);
            if (x + (s16)ww <= originX + w) {
                textFont.Draw((GE_MAIN == engine) ? (_bufferMain2 + _layerPitch) : _bufferSub2, x, y,
                            (const u8*)text, (GE_MAIN == engine) ? _penColor : _penColorSub);
            }
            text += add;
            x += ww;
        }
    }

    return x;
}

void cGdi::present(GRAPHICS_ENGINE engine) {
    if (GE_MAIN == engine) {
        if (_scheduleDrop) {
            dmaCopyWordsGdi(3, _bufferMain2, _bufferMain1, 256 * 192 * 2);
            swiWaitForVBlank();
            dmaCopyWordsGdi(3, _bufferMain2 + (256 * 192), _bufferMain3, 256 * 192 * 2);
            fillMemory((void*)_bufferMain2, 256 * 192 * 4, 0);
            _scheduleDrop = false;
        } else {
            dmaCopyWordsGdi(3, _bufferMain2 + _layerPitch, _mainEngineLayer == 0 ? _bufferMain1 : _bufferMain3, 256 * 192 * 2);
            fillMemory((void*)(_bufferMain2 + _layerPitch), 256 * 192 * 2, 0);
        }     

        oamUpdate(&oamMain);

    } else if (GE_SUB == engine) {
        if (SEM_GRAPHICS == _subEngineMode)
            dmaCopyWordsGdi(3, (void*)_bufferSub2, (void*)_bufferSub1, 256 * 192 * 2);
        fillMemory((void*)_bufferSub2, 0x18000, 0xffffffff);
    }
}

void cGdi::scheduleDrop() {
    _scheduleDrop = true;
}
